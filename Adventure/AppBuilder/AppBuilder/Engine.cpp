#include "Engine.h"

#include <functional>
#include <numeric>
#include <queue>

#include <input/keyboard.h>

#include "AdvDoc.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "Screenchange.h"
#include "Particles.h"
#include "Textout.h"
#include "Sound.h"
#include "PostProcessing.h"
#include "Menu.h"

using namespace adv;

TR_CHANNEL_LVL(ADV_Engine, TRACE_INFO);
//TR_CHANNEL_LVL(ADV_Engine_Room, TRACE_DEBUG);
TR_CHANNEL(ADV_Events);

namespace adv{
#ifndef WIN32

int _stricmp(const char* str1, const char* str2){
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int len = len1 < len2 ? len1 : len2;
  for (int i = 0; i < len; ++i){
    char ch1 = tolower(str1[i]);
    char ch2 = tolower(str2[i]);
    if (ch1 == ch2)
      continue;
    if (ch1 < ch2)
      return -1;
    return 1;
  }
  if (len1 == len2)
    return 0;
  return len1 < len2 ? -1 : 1;
}

#endif
}

Engine* Engine::mInstance = NULL;
static CGE::Mutex trymtx;

Engine::Engine() : mData(NULL), mInitialized(false), mWheelCount(0), mExitRequested(false), mResetRequested(false), mMenuShown(false), mTimeFactor(1.0f), mTimeFactorFaded(false),
  mSaver(NULL){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
  mAnimator = new Animator();
  mFonts = NULL;
  mInterpreter = NULL;
  mParticleEngine = NULL;
  for (int i = 0; i < 256; ++i){
    mKeysDown[i] = false;
    mKeysPressed[i] = false;
  }
}

Engine::~Engine(){
  delete mParticleEngine;
  delete mFonts;
  delete mAnimator;
  delete mInterpreter;
  delete mSaver;
}

void Engine::setData(AdvDocument* doc){
  mData = doc;
  delete mFonts;
  mInterpreter = new PcdkScript(mData);
  mFonts = new FontRenderer(mData);
  mSaver = new SaveStateProvider(mData);
  mParticleEngine = new ParticleEngine(mData);
}

void Engine::initGame(exit_callback exit_cb){
  if (!mData)
    return;
  if (exit_cb != NULL)
    mExitCall = exit_cb;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //ExecutionContext* ctx = mInterpreter->parseProgram("playswf (snitt ; 106 ;120 ;427 ; 330)");
  //mInterpreter->execute(ctx, true);
  mMainRoomLoaded = false;
  mSubRoomLoaded = false;
  mFocussedChar = NULL;
  Vec2i res = mData->getProjectSettings()->resolution;
  mWalkGridSize = res.x/32;
  mWalkFields = Vec2i(32,res.y/mWalkGridSize);
  //load cursor
  MouseCursor* cursor = mData->getCursor();
  mCursor = new CursorObject(Vec2i(0,0));
  for (unsigned j = 0; j < cursor->size(); ++j){
    Animation* anim = new Animation((*cursor)[j].frames, (*cursor)[j].fps, (*cursor)[j].highlight*-1, DEPTH_CURSOR);
    mCursor->addAnimation(anim, (*cursor)[j].command-1);
  }
  //load fonts
  mFontID = 1;
  mFonts->loadFont(0);
  mFonts->loadFont(mFontID);
  mActiveCommand = 0;
  mPrevActiveCommand = 0;
  mCurrentObject = NULL;
  mClickedObject = NULL;
  int transparency = mData->getProjectSettings()->anywhere_transparency*255/100;
  //load taskbar room
  if (mData->getProjectSettings()->show_taskbar && mData->getProjectSettings()->taskroom != ""){
    loadSubRoom(mData->getProjectSettings()->taskroom, NULL, 0);
    mRooms.front()->setOpacity(255-transparency);
    if (mData->getProjectSettings()->taskpopup != TB_SCROLLING)
      mTaskbar->setPosition(Vec2i(0,mData->getProjectSettings()->resolution.y-mData->getProjectSettings()->taskheight));
    else{
      if (mData->getProjectSettings()->taskHideCompletely)
        mTaskbar->setPosition(Vec2i(0,mData->getProjectSettings()->resolution.y));
      else
        mTaskbar->setPosition(Vec2i(0,mData->getProjectSettings()->resolution.y-20));
    }
  }
  else{
    mTaskbar = NULL;
  }
  //load anywhere room
  if (mData->getProjectSettings()->anywhere_room != ""){
    loadSubRoom(mData->getProjectSettings()->anywhere_room, NULL, 0);
    mRooms.front()->setOpacity(255-transparency);
  }
  //load main script
  Script* mainScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->mainscript);
  if (mainScript){
    mMainScript = mInterpreter->parseProgram(mainScript->text);
    mInterpreter->execute(mMainScript, false);
  }
  else
    mMainScript = NULL;
  //init some variables
  mUseObjectName = "";
  mGiveObjectName = "";
  mInitialized = true;
  mShowTaskbar = true;
  mScreenChange = mData->getProjectSettings()->screenchange;
  mTextEnabled = true;
  mUnloadedRoom = NULL;
  mForceNotToRenderUnloadingRoom = false;
  mPendingLoadRoom.roomName = "";
  mPendingLoadRoom.reason = NULL;
  mPendingLoadRoom.screenchange = SC_DIRECT;
  mRenderedMain = new RenderableBlitObject(mData->getProjectSettings()->resolution.x, mData->getProjectSettings()->resolution.y, 0);
  mRenderedMain->setBlendMode(BlitObject::BLEND_PREMULT_ALPHA);
  mPostProc = new PostProcessor(mData->getProjectSettings()->resolution.x, mData->getProjectSettings()->resolution.y, 0);
  mMouseShown = true;
  mMouseEnabled = true;
  mTimeFactor = 1.0f;
  mMenuEnabled = true;
  mFxShapesEnabled = true;
  mDraggingObject = NULL;
  mScrollSpeed = 0.0375f*4;
  mCamFollowChar = true;
  //load and execute start script
  Script* startScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->startscript);
  if (startScript){
    ExecutionContext* initScript = mInterpreter->parseProgram(startScript->text);
    mInterpreter->executeCutscene(initScript, false);
  }
}

void Engine::exitGame(){
  if (!mInitialized)
    return;
  mInitialized = false;
  mCurrentObject = NULL;
  mClickedObject = NULL;
  mInterpreter->stop();
  mMainScript->unref();
  mAnimator->clear();
  delete mUnloadedRoom;
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    mRooms.remove(*iter);
    delete *iter;
  }
  mRoomsToUnload.clear();
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    delete *iter;
  }
  mRooms.clear();
  clearGui();
  mMainRoomLoaded = false;
  resetCursor(true, false);
  delete mCursor;
  delete mFocussedChar;
  mFonts->unloadFont(mFontID);
  mFonts->unloadFont(0);
  mFonts->clearTextouts();
  mParticleEngine->activate(false, true);
  delete mRenderedMain;
  delete mPostProc;
  delete mDraggingObject;
  mDraggingObject = NULL;
}

CGE::Image* Engine::getImage(const std::string& name){
  if (name.empty())
    return NULL;
  if (name[0] == '#'){
    //get special images
    if (name == "#menu_bg"){
      return Menu::getBackground();
    }
    return NULL;
  }
  else
    return mData->getImage(name);
}

//TODO move into utility
unsigned Engine::roundToPowerOf2(unsigned x){
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

GLuint Engine::genTexture(const CGE::Image* image, Vec2i& size, Vec2f& scale, const CGE::Image* alphaimage){
  GLuint tex;
  if (!image)
    return 0;
  size.x = image->getWidth();
  size.y = image->getHeight();
  Vec2i pow2(roundToPowerOf2(size.x), roundToPowerOf2(size.y));
  scale.x = ((float)size.x)/pow2.x;
  scale.y = ((float)size.y)/pow2.y;
  GLint format = GL_RGBA;
  GLubyte* buffer;
  bool deleteBuffer = true;
  int numchannels = 4;
  if (image->getNumChannels() == 1){
    format = GL_LUMINANCE;
    buffer = image->getData();
    deleteBuffer = false;
    numchannels = 1;
  }
  else{
    unsigned totalsize = size.x*size.y;
    unsigned char* rgb = image->getData();
    //unsigned char* alpha = image.GetAlpha();
    buffer = new GLubyte[totalsize*4];
    int channels = image->getNumChannels();
    for (unsigned i = 0; i < totalsize; ++i){
      buffer[4*i] = rgb[channels*i];
      buffer[4*i+1] = rgb[channels*i+1];
      buffer[4*i+2] = rgb[channels*i+2];
      if (image->hasAlpha())
        buffer[4*i+3] = rgb[channels*i+3];
      else if (alphaimage){
        buffer[4*i+3] = alphaimage->getPixelChannel(i%size.x, i/size.x, 0);
      }
      else{
        if (rgb[channels*i] == 0xFF && rgb[channels*i+1] == 0x0 && rgb[channels*i+2] == 0xFF){
          buffer[4*i+3] = 0;
        }
        else{
          buffer[4*i+3] = 255;
        }
      }
    }
  }
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  char* init = new char[pow2.x*pow2.y*numchannels];
  memset(init, 0, pow2.x*pow2.y*numchannels);
  glTexImage2D(GL_TEXTURE_2D, 0, format, pow2.x, pow2.y, 0, format, GL_UNSIGNED_BYTE, init);
  delete [] init;
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  /*glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_R,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);*/
  if (deleteBuffer)
    delete buffer;
  return tex;
}

void Engine::render(unsigned time){
  if (mExitRequested){
    mExitCall();
    return;
  }
  if (mResetRequested){
    mMenuShown = false;
    Engine::instance()->setFocus("none", NULL);
    Engine::instance()->getSaver()->clear();
    Engine::instance()->exitGame();
    SoundEngine::instance()->reset();
    Engine::instance()->initGame(NULL);
    mResetRequested = false;
    return;
  }
  if (!mInitialized)
    return;
  //timing
  unsigned interval = (unsigned)(time*mTimeFactor);
  trymtx.lock();
  beginRendering();
  //unload rooms
  while (!mRoomsToUnload.empty()){
    //do not unload when a script is in progress
    if (mRoomsToUnload.front()->isScriptRunning()){
      //if (mInterpreter->isBlockingScriptRunning())
      mRoomsToUnload.front()->finishScripts(true);
      break;
    }
    /*ExecutionContext* ctx = mRoomsToUnload.front()->getScript();
    if (ctx){
      ctx->setEvent(EVT_EXIT);
      mInterpreter->executeImmediately(ctx);
    }*/
    if (mRoomsToUnload.front() == mRooms.back()){
      mMainRoomLoaded = false;
    }
    if (mRoomsToUnload.front() == mRooms.front()){
      mSubRoomLoaded = false;
    }
    mRooms.remove(mRoomsToUnload.front());

    if (mSaver->isWriteAllowed())
      mRoomsToUnload.front()->save();
    else
      mSaver->allowWrites();
    if (mUnloadedRoom)
      delete mUnloadedRoom;
    if (mForceNotToRenderUnloadingRoom){
      mUnloadedRoom = NULL;
      mForceNotToRenderUnloadingRoom = false;
      delete mRoomsToUnload.front();
    }
    else
      mUnloadedRoom = mRoomsToUnload.front();
    mRoomsToUnload.pop_front();
    if (mRoomsToUnload.empty() && !mPendingLoadRoom.roomName.empty()){
      //delayed load
      loadMainRoom(mPendingLoadRoom.roomName, mPendingLoadRoom.reason, mPendingLoadRoom.screenchange);
      if (mPendingLoadRoom.reason){
        mPendingLoadRoom.reason->resume();
        mPendingLoadRoom.reason = NULL;
      }
      mPendingLoadRoom.roomName = "";
    }
  }

  //do some scripting
  bool scriptupdate = mInterpreter->willUpdate(interval);
  if (scriptupdate){
    Object2D* obj = getObjectAt(mCursor->getPosition());
    if (obj != NULL){
      if (mCurrentObject != obj){
        if (mCurrentObject != NULL){
          ExecutionContext* script = mCurrentObject->getScript();
          if (script)
            script->setEvent(EVT_MOUSEOUT);
          TR_USE(ADV_Events);
          TR_DEBUG("mouseout on %s", mCurrentObject->getName().c_str());
          if (mDraggingObject == NULL)
            resetCursor(true, false);
        }
        mCurrentObject = obj;
        mObjectInfo.clear();
        mObjectTooltipInfo.clear();
        ExecutionContext* script = obj->getScript();
        if (script != NULL){
          TR_USE(ADV_Events);
          TR_DEBUG("mouse on %s", obj->getName().c_str());
          script->setEvent(EVT_MOUSE);
        }
      }
    }
    else{
      if (mCurrentObject){
        TR_USE(ADV_Events);
        ExecutionContext* script = mCurrentObject->getScript();
        if (script)
          script->setEvent(EVT_MOUSEOUT);
        TR_DEBUG("mouseout on %s", mCurrentObject->getName().c_str());
        if (mDraggingObject == NULL)
          resetCursor(true, false);
        mCurrentObject = NULL;
      }
      mObjectInfo.clear();
      mObjectTooltipInfo.clear();
    }

    if (!mInterpreter->isBlockingScriptRunning()){
      for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
        bool isUnloading = false;
        for (std::list<RoomObject*>::iterator iter2 = mRoomsToUnload.begin(); iter2 != mRoomsToUnload.end(); ++iter2){
          if (*iter2 == *iter){
            isUnloading = true;
            break;
          }
        }
        ExecutionContext* script = (*iter)->getScript();
        if (script != NULL && !isUnloading){
          script->setEvent(EVT_LOOP1);
          script->setEvent(EVT_LOOP2);
        }
      }
    }
    if (mRooms.size() > 0 && mFocussedChar && !mFocussedChar->isSpawnPos()/* && !mInterpreter->isBlockingScriptRunning()*/){ //walkmap
      Vec2i pos = mFocussedChar->getPosition()/mFocussedChar->getWalkGridSize();
      mRooms.back()->walkTo(pos);
    }

    //ui update
    for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
      while(!(*iter)->getScript()->getEvents().empty())
        mInterpreter->executeImmediately((*iter)->getScript());
    }
    clearGui();
  }

  mInterpreter->update(interval);

  //display tooltip
  if (!mObjectTooltipInfo.empty()){
    std::vector<Vec2i> breakinfo;
    Vec2i pos = Engine::instance()->getCursorPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(mObjectTooltipInfo, 1, breakinfo);
    Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2, pos.y-ext.y, mObjectTooltipInfo, DEPTH_GAME_FONT, 1, breakinfo, Engine::instance()->getSettings()->infotextcolor);
  }

  //can't all
  /*if (mFocussedChar){
    EngineEvent evt = mFocussedChar->getScript()->getCommandEvent();
    if (evt >= EVT_USER_MIRROR_BEGIN && evt <= EVT_USER_MIRROR_END){
      mFocussedChar->getScript()->resetEvent(evt);
      mFocussedChar->getScript()->setEvent(EVT_CANT_ALL);
    }
  }*/

  //scrolling
  if (mRooms.size() > 0 && mFocussedChar && !mInterpreter->isBlockingScriptRunning() && mFocussedChar->getRoom() == mRooms.back()->getName() && mCamFollowChar){
    mScrollOffset = mData->getProjectSettings()->resolution/2-
      (mFocussedChar->getPosition()-Vec2i(0,mFocussedChar->getSize().y/2));
    //mRooms.back()->setScrollOffset(mScrollOffset); //this function limits the scrolling
    mAnimator->add(mRooms.back(), mScrollOffset, mScrollSpeed);
    mScrollOffset = mRooms.back()->getScrollOffset();
    //mFocussedChar->setScrollOffset(mScrollOffset);
  }

  //some animation stuff
  mCursor->update(interval);
  mAnimator->update(interval);
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    (*iter)->update(interval);
  }
  if (mFocussedChar && mFocussedChar->getAnimation())
    mFocussedChar->update(interval);
  if (mParticleEngine->getDepth() == DEPTH_PARTICLES_TOP)
    mParticleEngine->update(interval);

  if (!mTextEnter.empty()){
    mBlinkTimeAccu += interval;
    while (mBlinkTimeAccu > 500){
      mBlinkTimeAccu -= 500;
      mBlinkCursorVisible = !mBlinkCursorVisible;
    }
    if (mBlinkCursorVisible){
      std::string text = mInterpreter->getVariable(mTextEnter).getString();
      mInterpreter->setVariable(mTextEnter, text+"_");
    }
  }

   mFonts->prepareTextouts();

  if (!mTextEnter.empty() && mBlinkCursorVisible){
    std::string text = mInterpreter->getVariable(mTextEnter).getString();
    text.erase(text.size()-1);
    mInterpreter->setVariable(mTextEnter, text);
  }

  //build blit queue
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int roomdepth = 0;
  RoomObject* mainroom = NULL;
  for (std::list<RoomObject*>::reverse_iterator iter = mRooms.rbegin(); iter != mRooms.rend(); ++iter){
    (*iter)->setDepth(roomdepth);
    roomdepth += DEPTH_ROOM_RANGE;
    if ((*iter) == mTaskbar){
      if (!mShowTaskbar || mInterpreter->isBlockingScriptRunning())
        continue;
    }
    if ((*iter)->getName() == mData->getProjectSettings()->anywhere_room){
      if (mInterpreter->isBlockingScriptRunning())
        continue;
    }
    if (mMainRoomLoaded && iter == mRooms.rbegin()){
      mainroom = (*iter);
      mRenderedMain->bind();
      beginRendering();
      mFonts->prepareBlit(interval, mainroom, true);
      //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      GL()pushMatrix();
      GL()loadIdentity();
      GL()translatef(0.0f, (float)Engine::instance()->getResolution().y, 0.0f);
      GL()scalef(1.0f,-1.0f,1.0f);
    }
    (*iter)->render();
    if (mMainRoomLoaded && iter == mRooms.rbegin()){
      if (mFocussedChar && mFocussedChar->getRoom() == (*iter)->getName())
        mFocussedChar->render();
      if (mParticleEngine->getDepth() != DEPTH_PARTICLES_TOP) //render the particles now if they belong to the main room
        mParticleEngine->update(interval);
      endRendering();
      GL()popMatrix();
      mRenderedMain->unbind();
      BlitObject* result = mPostProc->process(mRenderedMain, interval);
      result->render(Vec2i(), Vec2f(1.0f,1.0f), Vec2i());
    }
    //This was here to hide taskbar and anywhere room. Replaced by continue statements above, so that other subrooms remain.
    //if (mInterpreter->isBlockingScriptRunning())
    //  break;
  }
  if ((!mInterpreter->isBlockingScriptRunning() || mInterpreter->isTextScene()) && mMouseShown && mMouseEnabled){
    mCursor->render();
    if (mDraggingObject != NULL){
      mDraggingObject->setPosition(mCursor->getPosition()+mCursor->getSize()/2);
      mDraggingObject->render();
    }
  }
  for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
    (*iter)->render();
  }

  //command handling
  Vec2i res = mData->getProjectSettings()->resolution;
  std::string text;
  if (mFocussedChar){
    int cmdidx;
    if ((mUseObjectName.empty() && mGiveObjectName.empty()) || mPrevActiveCommand == 0)
      cmdidx = mActiveCommand;
    else
      cmdidx = mPrevActiveCommand;
    int langcmdidx = cmdidx;
    if (langcmdidx > 0)
      langcmdidx += 1;
    if (mData->hasLanguageInfo())
      text = mData->getLanguageString(getInterpreter()->getLanguage(), Language::COMMANDS, langcmdidx);
    else
      text = mData->getProjectSettings()->pretty_commands[cmdidx];
    if (!mUseObjectName.empty()){
      text += " "+mLinkObjectInfo;
      if (mData->hasLanguageInfo())
        text += " "+mData->getLanguageString(getInterpreter()->getLanguage(), Language::COMMANDS, 1);
      else
        text += " "+mData->getProjectSettings()->linktext;
    }
    if (!mGiveObjectName.empty()){
      text += " "+mLinkObjectInfo;
      if (mData->hasLanguageInfo())
        text += " "+mData->getLanguageString(getInterpreter()->getLanguage(), Language::COMMANDS, 2);
      else
        text += " "+mData->getProjectSettings()->givelink;
    }
    if (!mObjectInfo.empty()){
      text += " "+mObjectInfo;
    }
  }
  mActionText = text;
  if (mData->getProjectSettings()->show_actiontext){
    //elevate the action line
    if (mTaskbar && !mInterpreter->isBlockingScriptRunning() && mShowTaskbar){
      res.y = mTaskbar->getPosition().y;
    }
    std::vector<Vec2i> breakinfo;
    Vec2i offset = mFonts->getTextExtent(text, 1, breakinfo);
    if (!mInterpreter->isBlockingScriptRunning() && mFocussedChar != NULL)
      mFonts->render(res.x/2-offset.x/2, res.y-offset.y, text, DEPTH_GAME_FONT, 1, breakinfo, Engine::instance()->getSettings()->infotextcolor);
  }

  mFonts->prepareBlit(interval, mainroom, false);

  //render the stuff
  endRendering();
  trymtx.unlock();
}

bool Engine::loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading){
  TR_USE(ADV_Engine);
  if (!mData)
    return false;
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if (_stricmp((*iter)->getName().c_str(), name.c_str()) == 0){
      bool isUnloading = false;
      for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
        if (_stricmp((*iter)->getName().c_str(), name.c_str()) == 0){
          isUnloading = true;
          break;
        }
      }
      if (!isUnloading)
        return true;
    }
  }
  if (mMainRoomLoaded && !isSubRoom){
    TR_INFO("requested loading of room %s", name.c_str());
    unloadRoom(mRooms.back(), true, false);
    if (mPendingLoadRoom.reason != NULL){
      mPendingLoadRoom.reason->resume();
    }
    if (loadreason)
      loadreason->suspend(0, NULL);
    mPendingLoadRoom.roomName = name;
    mPendingLoadRoom.reason = loadreason;
    mPendingLoadRoom.screenchange = change;
    return false;
  }
  TR_INFO("loading room %s with screenchange %i", name.c_str(), change);
  Room* room = mData->getRoom(name);
  SaveStateProvider::SaveRoom* save = mSaver->getRoom(room->name);
  if (!room || !save)
    return false;
  int depthoffset = 0;
  if (isSubRoom){
    depthoffset = (int)mRooms.size()*1000;
    if (!mMainRoomLoaded)
      depthoffset += 1000;
  }
  Room* rm = mData->getRoom(name);
  RoomObject* roomobj = new RoomObject(save->base.state, /*save->base.position*/Vec2i(0,0), room->size, room->name, rm->depthmap, rm->doublewalkmap);
  roomobj->setParallaxBackground(room->parallaxbackground, depthoffset+DEPTH_PARALLAX_BACKGROUND);
  //anywhere room is not allowed to have background
  if (_stricmp(room->name.c_str(), mData->getProjectSettings()->anywhere_room.c_str()) != 0)
    roomobj->setBackground(room->background, depthoffset+DEPTH_BACKGROUND);
  roomobj->setLightingColor(save->overlaylighting);
  roomobj->setOpacity(save->base.lighting.a);
  roomobj->setWalkmap(room->walkmap);
  for (std::map<Vec2i,bool>::iterator iter = save->walkmap.begin(); iter != save->walkmap.end(); ++iter){
    roomobj->modifyWalkmap(iter->first, iter->second);
  }
  roomobj->setScrollOffset(save->scrolloffset);
  roomobj->setZoomFactor(rm->zoom);
  //check for walkmap scripts
  std::vector<std::pair<Vec2i,Script*> > wmscripts = mData->getWMScripts(room->name);
  for (unsigned i = 0; i < wmscripts.size(); ++i){
    ExecutionContext* scr = mInterpreter->parseProgram(wmscripts[i].second->text);
    if (scr != NULL)
      roomobj->addWalkmapScript(wmscripts[i].first-Vec2i(1,1), scr);
  }
  for (unsigned i = 0; i < room->objects.size(); ++i){
    Object* o = mData->getObject(room->objects[i].object);
    SaveStateProvider::SaveObject* saveobj = mSaver->getObject(room->objects[i].name);
    if (!saveobj)
      continue;
    Object2D* object = new Object2D(saveobj->state, saveobj->position, o->size, room->objects[i].name);
    object->setLightingColor(saveobj->lighting);
    //calculate render depth
    int depth;
    if (room->objects[i].layer == 0)
      depth = DEPTH_BACK_OBJECTS;
    else if (room->objects[i].layer == 1)
      depth = room->objects[i].wm_depth-1;
    else
      depth = DEPTH_FRONT_OBJECTS;
    object->setDepth(depth+depthoffset);
    //object->setScale(roomobj->getDepthScale(saveobj->position));
    for (unsigned j = 0; j < o->states.size(); ++j){
      Animation* anim = new Animation(o->states[j].frames, o->states[j].fps, depth+depthoffset);
      object->addAnimation(anim);
    }
    object->setLighten(o->lighten);
    //check for object scripts
    Script* script = mData->getScript(Script::OBJECT,room->objects[i].name+";"+room->name);
    if (script){
      ExecutionContext* scr = mInterpreter->parseProgram(script->text);
      if (scr){
        object->setScript(scr);
        mInterpreter->execute(scr, false);
      }
    }
    roomobj->addObject(object);
  }
  //now check for the characters
  for (std::map<std::string, SaveStateProvider::CharSaveObject*>::iterator iter = save->characters.begin(); iter != save->characters.end(); ++iter){
    Rcharacter ch;
    for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
      if (mData->getRoomCharacters()[i].name == iter->first){
        ch = mData->getRoomCharacters()[i];
        break;
      }
    }
    CharacterObject* character = loadCharacter(ch.name, ch.character, false, loadreason);
    if (character){
      character->setScale(roomobj->getDepthScale(character->getPosition()));
      roomobj->addObject(character);
    }
  };
  //load mirror objects
  for (unsigned i = 0; i < rm->fxshapes.size(); ++i){
    if (rm->fxshapes[i].active){
      if (rm->fxshapes[i].effect == FXShape::WALL_MIRROR || rm->fxshapes[i].effect == FXShape::FLOOR_MIRROR){
        int depth = rm->fxshapes[i].effect == FXShape::FLOOR_MIRROR ? DEPTH_MIRROR_BACK : rm->fxshapes[i].depth-1+depthoffset;
        MirrorObject* mirror = new MirrorObject(mData->getProjectSettings()->resolution.x, mData->getProjectSettings()->resolution.y, depth, rm->fxshapes[i].strength);
        mirror->setMirrorArea(rm->fxshapes[i].points, roomobj);
        if (rm->fxshapes[i].effect == FXShape::WALL_MIRROR){
          mirror->setWallMirror(rm->fxshapes[i].mirrorOffset, rm->fxshapes[i].dependingOnRoomPosition);
        }
        roomobj->addMirror(mirror);
      }
      else if (rm->fxshapes[i].effect == FXShape::PARTICLE_BARRIER){
        roomobj->addBarrier(rm->fxshapes[i].points);
      }
    }
  }
  roomobj->Object2D::setPosition(save->base.position); //correct room position only now
  //load room script
  Script* script = mData->getScript(Script::ROOM,room->name);
  if (script){
    ExecutionContext* scr = mInterpreter->parseProgram(script->text);
    if (scr != NULL){
      roomobj->setScript(scr);
      mInterpreter->execute(scr, false);
      scr->setEvent(EVT_ENTER);
      //mInterpreter->executeImmediately(scr);
    }
  }
  //inventory display
  if (room->invsize != Vec2i()){
    InventoryDisplay* disp = new InventoryDisplay(room->invpos, room->invsize, room->invscale, DEPTH_ITEM+depthoffset);
    roomobj->setInventory(disp);
  }

  if (isSubRoom){
    mRooms.push_front(roomobj);
    if (mInitialized)
      mSubRoomLoaded = true;
  }
  else{
    mRooms.push_back(roomobj);
    mMainRoomLoaded = true;
  }
  if (_stricmp(room->name.c_str(), mData->getProjectSettings()->taskroom.c_str()) == 0)
    mTaskbar = roomobj;
  if (mFocussedChar && mFocussedChar->getRoom() == roomobj->getName()){
    mFocussedChar->setScale(roomobj->getDepthScale(mFocussedChar->getPosition()));
    mFocussedChar->setScrollOffset(roomobj->getScrollOffset());
    mFocussedChar->setDepth(mFocussedChar->getPosition().y/Engine::instance()->getWalkGridSize(false));
  }
  //animation stuff
  if (!isSubRoom)
    triggerScreenchange(loadreason, change, false);
  else if (fading != 0){
    roomobj->setFadeout(fading);
    mAnimator->add(roomobj, fading, true);
  }
  return true;
}

void Engine::unloadRoom(RoomObject* room, bool mainroom, bool immediately){
  if (mRooms.empty())
    return;
  if (room == NULL){
    if (mainroom){
      room = mRooms.back();
    }
    else{
      mMenuShown = false; //resets the menu shown flag when subroom is unloaded
      room = mRooms.front();
    }
  }
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    if (*iter == room)
      return; //unload already in progress
  }
  mRoomsToUnload.push_back(room);
  if (mCurrentObject){
    mCurrentObject->getScript()->setEvent(EVT_MOUSEOUT);
    delete mDraggingObject;
    mDraggingObject = NULL;
    resetCursor(true, false);
  }
  mCurrentObject = NULL;
  ExecutionContext* script = room->getScript();
  if (script)
    script->setEvent(EVT_EXIT);
  room->finishScripts(false);
  //room->save();
  if (immediately){
    room->setFadeout(0);
    for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
      if (*iter == room){
        if (room == mRooms.back()){
          mMainRoomLoaded = false;
        }
        if (room == mRooms.front()){
          mSubRoomLoaded = false;
        }
        mRooms.erase(iter);
        break;
      }
    }
  }
  else{
    if (!mainroom){
      if (room->getFadeout() > 0){
        mAnimator->add(room, room->getFadeout(), false);
      }
    }
  }
}

void Engine::beginRendering(){
  mBlitQueues.push_back(std::list<BaseBlitObject*>());
}

void Engine::endRendering(){
  restoreRenderDefaults();
  for (std::list<BaseBlitObject*>::iterator iter = mBlitQueues.back().begin(); iter != mBlitQueues.back().end(); ++iter){ 
    (*iter)->blit();
  }
  mBlitQueues.pop_back();
}

void Engine::insertToBlit(BaseBlitObject* obj){
  bool placenotfound = true;
  for (std::list<BaseBlitObject*>::iterator iter = mBlitQueues.back().begin(); iter != mBlitQueues.back().end(); ++iter){
    if (obj->getDepth() <= (*iter)->getDepth()){
      mBlitQueues.back().insert(iter, obj);
      placenotfound = false;
      break;
    }
  }
  if (placenotfound)
    mBlitQueues.back().push_back(obj);
}

void Engine::setCursorPos(Vec2i pos){
  trymtx.lock();
  mCursor->setPosition(pos);
  //triggered scrolling
  if (mTaskbar && pos.y < mData->getProjectSettings()->resolution.y - mData->getProjectSettings()->taskheight - 16){
    if (mData->getProjectSettings()->taskpopup == TB_SCROLLING){
      std::list<Vec2i> target;
      if (mData->getProjectSettings()->taskHideCompletely)
        target.push_back(Vec2i(0,mData->getProjectSettings()->resolution.y));
      else
        target.push_back(Vec2i(0,mData->getProjectSettings()->resolution.y-20));
      mAnimator->add(mTaskbar, target, 10);
    }
    if (mData->getProjectSettings()->taskpopup == TB_DIRECT){
      mTaskbar->setPosition(Vec2i(0,mData->getProjectSettings()->resolution.y));
    }
  }
  if (mTaskbar && pos.y > mData->getProjectSettings()->resolution.y - 16){
    if (mData->getProjectSettings()->taskpopup == TB_SCROLLING){
      //mTaskbar->setPosition(Vec2i(0,mData->getProjectSettings()->resolution.y-mData->getProjectSettings()->taskheight));
      std::list<Vec2i> target;
      target.push_back(Vec2i(0,mData->getProjectSettings()->resolution.y-mData->getProjectSettings()->taskheight));
      mAnimator->add(mTaskbar, target, 10);
    }
    if (mData->getProjectSettings()->taskpopup == TB_DIRECT){
      mTaskbar->setPosition(Vec2i(0,mData->getProjectSettings()->resolution.y-mData->getProjectSettings()->taskheight));
    }
  }
  trymtx.unlock();
}

Vec2i Engine::getCursorPos(){
  return mCursor->getPosition();
}

void Engine::leftClick(const Vec2i& pos){
  if (!mMouseEnabled)
    return;
  trymtx.lock();
  ExecutionContext* script = NULL;
  Object2D* obj = getObjectAt(pos);
  bool keepCommand = false;
  if (obj != NULL){
    if (obj->getScript() != NULL && !obj->getScript()->isGameObject())
      keepCommand = true;
    mClickedObject = obj;
    script = obj->getScript();
    if (script != NULL){
      script->setEvent(EVT_CLICK);
      if (!mUseObjectName.empty())
        script->setEvent(EVT_LINK);
      else if (!mGiveObjectName.empty())
        script->setEvent(EVT_GIVE_LINK);
      else
        script->setEvent((EngineEvent)mActiveCommand);
    }
    else if (mFocussedChar && !mSubRoomLoaded){
      Engine::instance()->walkTo(mFocussedChar, pos-mScrollOffset, UNSPECIFIED, 1.0f);
    }
  }
  else if (mFocussedChar && !mSubRoomLoaded && !mInterpreter->isBlockingScriptRunning()){
    Engine::instance()->walkTo(mFocussedChar, pos-mScrollOffset, UNSPECIFIED, 1.0f);
  }
  if (!keepCommand){
    int curCmd = mCursor->getCurrentCommand();
    if (curCmd != mActiveCommand && mLinkObjectInfo.empty()){
      mPrevActiveCommand = mActiveCommand;
      mActiveCommand = curCmd;
      mUseObjectName = "";
      mGiveObjectName = "";
      mLinkObjectInfo = "";
      resetCursor(true, true);
    }
  }
  trymtx.unlock();
}

void Engine::leftRelease(const Vec2i& pos){
  if (!mMouseEnabled)
    return;
  trymtx.lock();
  if (mClickedObject != NULL){
    ExecutionContext* ctx = mClickedObject->getScript();
    if (ctx != NULL)
      ctx->setEvent(EVT_RELEASE);
    mClickedObject = NULL;
  }
  trymtx.unlock();
}

void Engine::rightClick(const Vec2i& pos){
  if (!mMouseEnabled)
    return;
  bool leftClickRequired;
  int cmd = mCursor->getNextCommand(leftClickRequired, pos);
  mActiveCommand = cmd;
  mUseObjectName = "";
  mGiveObjectName = "";
  mLinkObjectInfo = "";
  if (leftClickRequired)
    leftClick(pos);
  else{
    Object2D* obj = getObjectAt(pos);
    if (obj != NULL){
      ExecutionContext* script = obj->getScript();
      if (script != NULL)
        script->setEvent(EVT_RIGHTCLICK);
    }
  }
}

void Engine::doubleClick(const Vec2i& pos){
  if (!mMouseEnabled)
    return;
  ExecutionContext* script = NULL;
  Object2D* obj = getObjectAt(pos);
  if (obj != NULL){
    script = obj->getScript();
    if (script != NULL){
      script->setEvent(EVT_DBLCLCK);
    }
  }
}

void Engine::mouseWheel(int delta){
  if (!mMouseEnabled)
    return;
  mWheelCount += delta;
}

bool Engine::setFocus(std::string charname, ExecutionContext* reason){
  CharacterObject* deletionChar = NULL;
  if (mFocussedChar){
    if (mFocussedChar->getRoom() == mRooms.back()->getName())
      mRooms.back()->addObject(mFocussedChar);
    else{
      //just store the character as it is in another room
      mSaver->getRoom(mFocussedChar->getRoom());
      mFocussedChar->save();
      deletionChar = mFocussedChar;
    }
  }
  if (charname == "none"){
    if (mFocussedChar)
      mLastFocussedChar = mFocussedChar->getName();
    mFocussedChar = NULL;
    delete deletionChar;
    return true;
  }
  if (charname == "last"){
    charname = mLastFocussedChar;
  }
  CharacterObject* res = extractCharacter(charname);
  if (res){
    mFocussedChar = res;
    delete deletionChar;
    return true;
  }
  /*if (!mLastFocussedChar.empty() && mLastFocussedChar != "none"){
    focusChar(mLastFocussedChar);
    return true;
  }*/
  //load character
  res = loadCharacter(charname, getCharacterClass(charname), true, reason);
  if (res){
    SaveStateProvider::SaveRoom* rm = getSaver()->getRoom(res->getRoom());
    mSaver->removeCharacter(res->getName());
    res->setScrollOffset(rm->scrolloffset);
    mFocussedChar = res;
    delete deletionChar;
    return true;
  }
  else{
    TR_USE(ADV_Engine);
    TR_BREAK("Character %s not found", charname.c_str());
  }
  //mCharOutOfFocus = true;
  return false;
}

bool Engine::aStarSearch(const Vec2i& from, const Vec2i& to, std::list<Vec2i>& path){
  std::set<AStarData> closedset;
  std::priority_queue<AStarData, std::vector<AStarData>, std::greater<AStarData> > openset;
  std::set<AStarData> openset2;
  AStarData min_h_score;
  AStarData start;
  start.pos = from;
  start.g_score = 0;
  start.h_score = distance(from, to);
  start.f_score = start.h_score;
  start.parent = from;
  min_h_score = start;
  openset.push(start);
  openset2.insert(start);
  while(!openset.empty()){
    AStarData x = openset.top();
    if (x.pos == to){
      //goal reached
      path = reconstruct_path(x, closedset);
      return true;
    }
    if (x.h_score < min_h_score.h_score)
      min_h_score = x;
    openset.pop();
    openset2.erase(x);
    closedset.insert(x);
    for (int i = 0; i < 4; ++i){
      //calculate neighbour
      AStarData y;
      y.pos = x.pos;
      switch(i){
        case 0:
          y.pos.y -= 1;
          if (y.pos.y < 0)
            continue;
          break;
        case 1:
          y.pos.y += 1;
          if (y.pos.y >= mData->getProjectSettings()->resolution.y/mWalkGridSize*4)
            continue;
          break;
        case 2:
          y.pos.x -= 1;
          if (y.pos.x < 0)
            continue;
          break;
        case 3:
          y.pos.x += 1;
          if (y.pos.x >= mData->getProjectSettings()->resolution.x/mWalkGridSize*6)
            continue;
          break;
      }
      if (!mRooms.back()->isWalkable(y.pos))
        continue;
      if (closedset.find(y) != closedset.end())
        continue;
      float tentative_g_score = x.g_score + 1/*distance constant 1 due to grid layout*/;
      if (openset2.find(y) == openset2.end()){
        y.h_score = distance(y.pos, to);
        y.parent = x.pos;
        y.g_score = tentative_g_score;
        y.f_score = y.g_score + y.h_score;
        openset.push(y);
        openset2.insert(y);
      }
      else if (tentative_g_score < y.g_score){
        y.parent = x.pos;
        y.g_score = tentative_g_score;
        y.f_score = y.g_score + y.h_score;
      }
    }
  }
  path = reconstruct_path(min_h_score, closedset);
  return false;
}

std::list<Vec2i> Engine::reconstruct_path(AStarData node, const std::set<AStarData>& data){
  std::list<Vec2i> result;
  result.push_front(node.pos);
  while (node.parent != node.pos){
    AStarData cmp;
    cmp.pos = node.parent;
    std::set<AStarData>::const_iterator iter = data.find(cmp);
    if (iter == data.end())
      return result;
    node = *iter;
    result.push_front(node.pos);
  }
  return result;
}

float Engine::distance(const Vec2i& x, const Vec2i& y){
  Vec2i dir = y-x;
  //manhattan distance
  return fabs((float)dir.x)+fabs((float)dir.y);
}

Object2D* Engine::getObjectAt(const Vec2i& pos){
  for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
    if ((*iter)->isHit(pos)){
      return (*iter);
    }
  }
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    Object2D* ret = (*iter)->getObjectAt(pos);
    if (ret != NULL)
      return ret;
    if (mSubRoomLoaded)
      break;
  }
  return NULL;
}

Object2D* Engine::getObject(const std::string& name, bool searchInventoryFirst){
  if (mFocussedChar && searchInventoryFirst){
    Object2D* ret = mFocussedChar->getInventory()->getItem(name);
    if (ret != NULL)
      return ret;
  }
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    Object2D* ret = (*iter)->getObject(name);
    if (ret != NULL)
      return ret;
  }
  //also rooms to unload need check
  //no, this leads to invalid objects
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    Object2D* ret = (*iter)->getObject(name);
    if (ret != NULL)
      return ret;
  }
  if (mFocussedChar && !searchInventoryFirst){
    Object2D* ret = mFocussedChar->getInventory()->getItem(name);
    if (ret != NULL)
      return ret;
  }
  for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
    if (_stricmp((*iter)->getName().c_str(), name.c_str()) == 0)
      return *iter;
  }
  return NULL;
}

CharacterObject* Engine::getCharacter(const std::string& name){
  if (_stricmp(name.c_str(), "self") == 0)
    return mFocussedChar;
  if (mFocussedChar && _stricmp(mFocussedChar->getName().c_str(), name.c_str()) == 0)
    return mFocussedChar;
  CharacterObject* res = NULL;
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    res = (*iter)->findCharacter(name);
    if (res){
      return res;
    }
  }
  return NULL;
}

CharacterObject* Engine::extractCharacter(const std::string& name){
  CharacterObject* res = NULL;
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    res = (*iter)->extractCharacter(name);
    if (res){
      SaveStateProvider::SaveRoom* srm = mSaver->getRoom((*iter)->getName());
      mSaver->removeCharacter(res->getName());
      return res;
    }
  }
  return NULL;
}

RoomObject* Engine::getRoom(const std::string& name){
  if (name.empty()){
    if (mMainRoomLoaded)
      return mRooms.back();
    return NULL;
  }
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if (_stricmp((*iter)->getName().c_str(), name.c_str()) == 0){
      return *iter;
    }
  }
  return NULL;
}

void Engine::walkTo(CharacterObject* chr, const Vec2i& pos, LookDir dir, float speedFactor){
  int walkgridsize = chr->getWalkGridSize();
  Vec2i oldwmpos = chr->getPosition()/walkgridsize;
  Vec2i newwmpos = pos/walkgridsize;
  std::list<Vec2i> path;
  bool couldReach = aStarSearch(oldwmpos,newwmpos,path); 
  //remove first entry as we are already on that field
  path.pop_front();
  for (std::list<Vec2i>::iterator iter = path.begin(); iter != path.end(); ++iter){
    *iter = *iter*walkgridsize+Vec2i(walkgridsize/2,walkgridsize/2);
  }
  //replace last entry
  if (couldReach){
    if (!path.empty())
      path.pop_back();
    path.push_back(pos);
  }
  chr->setEndLookDir(dir);
  Character* ch = mData->getCharacter(getCharacterClass(chr->getName()));
  if (!ch){
    TR_USE(ADV_Engine);
    TR_BREAK("Character not found");
  }
  mAnimator->add(chr, path, (10-ch->walkspeed)*speedFactor);
}

Object2D* Engine::createItem(const std::string& name){
  Item* it = mData->getItem(name);
  if (it == NULL){
    TR_USE(ADV_Engine);
    TR_BREAK("Item %s not found", name.c_str());
    return NULL;
  }
  Object2D* object = new Object2D(1, Vec2i(), Vec2i(50,50), it->name);
  int depth = DEPTH_ITEM;
  for (unsigned j = 0; j < it->states.size(); ++j){
    Animation* anim = new Animation(it->states[j].frames, it->states[j].fps, Vec2i(), depth);
    object->addAnimation(anim);
  }
  //check for object scripts
  Script* script = mData->getScript(Script::ITEM,it->name);
  if (script){
    ExecutionContext* scr = mInterpreter->parseProgram(script->text);
    object->setScript(scr);
    mInterpreter->execute(scr, false);
  }
  return object;
}

void Engine::setUseObject(const std::string& object, const std::string& objectInfo){
  mLinkObjectInfo = objectInfo;
  mUseObjectName = object;
  if (mData->getProjectSettings()->draw_dragged_items){
    delete mDraggingObject;
    if (object.empty())
      mDraggingObject = NULL;
    else{
      mDraggingObject = createItem(mClickedObject->getName());
      mDraggingObject->setDepth(DEPTH_CURSOR-1);
    }
  }
}

void Engine::setGiveObject(const std::string& object, const std::string& objectInfo){
  mLinkObjectInfo = objectInfo;
  mGiveObjectName = object;
}

ExecutionContext* Engine::loadScript(Script::Type type, const std::string& name){
  Script* scr = mData->getScript(type, name);
  ExecutionContext* ctx = mInterpreter->parseProgram(scr->text);
  return ctx;
}

void Engine::addUIElement(Object2D* elem, int offset){
  if (offset != 0){
    for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
      Vec2i pos = (*iter)->getPosition();
      pos.y += offset;
      (*iter)->setPosition(pos);
    }
  }
  mUI.push_back(elem);
}

void Engine::setCommand(const std::string& command, bool deleteLinks){
  if (command == "none"){
    int cmd = mData->getProjectSettings()->commands["walkto"];
    mActiveCommand = cmd;
    mCursor->setState(1);
  }
  else{
    int cmd = mData->getProjectSettings()->commands[command];
    mActiveCommand = cmd;
    mCursor->setCommand(cmd);
  }
  if (deleteLinks){
    mUseObjectName = "";
    mGiveObjectName = "";
    mLinkObjectInfo = "";
    resetCursor(true, true);
  }
}

std::string Engine::getActiveCommand(){
  for (std::map<std::string, unsigned int>::iterator iter = mData->getProjectSettings()->commands.begin();
    iter != mData->getProjectSettings()->commands.end(); ++iter){
      if (iter->second == mActiveCommand)
        return iter->first;
  }
  return "";
}

void Engine::clearGui(){
  for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
    /*Object2D* obj = *iter;
    ExecutionContext* ctx = obj->getScript();
    if (ctx){
      if (!ctx->getEvents().empty())
        TR_BREAK();
    }*/
    delete (*iter);
  }
  mUI.clear();
}

RoomObject* Engine::getContainingRoom(Object2D* object){
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if ((*iter)->containsObject(object))
      return *iter;
  }
  return NULL;
}

CharacterObject* Engine::loadCharacter(const std::string& instanceName, const std::string& className, bool loadContainingRoom, ExecutionContext* loadreason){
  SaveStateProvider::CharSaveObject* obj = NULL;
  std::string room;
  std::string realName;
  if (loadContainingRoom){
    obj = mSaver->findCharacter(instanceName, room, realName);
    if (obj){
      loadMainRoom(room, loadreason, Engine::instance()->getScreenChange());
      CharacterObject* chr = extractCharacter(realName);
      if (chr)
        return chr;
    }
  }
  if (mFocussedChar && _stricmp(mFocussedChar->getName().c_str(), instanceName.c_str()) == 0)
    return NULL;
  if (!obj){
    obj = mSaver->findCharacter(instanceName, room, realName);
  }
  Character* chbase = mData->getCharacter(className);
  if (chbase == NULL)
    return NULL;
  CharacterObject* character = new CharacterObject(chbase, obj->base.state, obj->mirrored, obj->base.position, realName);
  character->setLightingColor(obj->base.lighting);
  character->setFontID(obj->fontid);
  mFonts->loadFont(obj->fontid);
  character->setUserScale(obj->scale);
  character->setTextColor(chbase->textcolor);
  character->setRoom(room);
  character->setNoZooming(obj->nozooming, true);
  if (!obj->walksound.empty()){
    SoundPlayer* plyr = SoundEngine::instance()->getSound(obj->walksound, SoundEngine::PLAYER_CREATE_ALWAYS | SoundEngine::PLAYER_UNMANAGED);
    character->setWalkSound(plyr);
  }
  if (!obj->linkedObject.empty()){
    Object2D* link = getObject(obj->linkedObject, false);
    character->setLinkObject(link);
  }
  //RoomObject* ro = Engine::instance()->getRoom(room);
  //if (ro)
  //  character->setScale(ro->getDepthScale(obj->base.position));
  for (unsigned j = 0; j < chbase->states.size(); ++j){
    int depth = (obj->base.position.y+chbase->states[j].basepoint.y)/Engine::instance()->getWalkGridSize(false);
    Animation* anim = new Animation(chbase->states[j].frames, chbase->states[j].fps, depth);
    character->addAnimation(anim);
    character->addBasepoint(chbase->states[j].basepoint, chbase->states[j].size);
  }
  Script* script = mData->getScript(Script::CHARACTER,instanceName);
  if (script){
    ExecutionContext* scr = mInterpreter->parseProgram(script->text);
    if (scr){
      character->setScript(scr);
      mInterpreter->execute(scr, false);
    }
  }
  for (std::map<int,std::vector<std::string> >::iterator inviter = obj->inventory.items.begin();
    inviter != obj->inventory.items.end(); ++inviter){
      for (unsigned i = 0; i < inviter->second.size(); ++i){
        Object2D* item = createItem(inviter->second[i]);
        character->getInventory()->addItem(item, inviter->first);
      }
  }
  character->getInventory()->setCurrent(obj->inventory.current);
  return character;
}

void Engine::keyPress(int key){
  if (mTextEnter.empty()){
    mKeysPressed[key] = true;
    mKeysDown[key] = true;
  }
  switch(key){
    case KEY_F1:
      mSaver->save(SaveStateProvider::saveSlotToPath(0));
      break;
    case KEY_F2:
      mSaver->load(SaveStateProvider::saveSlotToPath(0));
      break;
    case KEY_ESCAPE:
      {
        ExecutionContext* ctx = mInterpreter->getCutscene();
        if (ctx){
          ctx->setSkip();
        }
        else if (!mTextEnter.empty()){
          mInterpreter->resumeBlockingScript();
          mTextEnter.clear();
          mSuspender->resume();
          mFonts->getTextout(-1)->setEnabled(false);
        }
        else if (mMenuEnabled){
          if (!mMenuShown){
            if (mData->getProjectSettings()->has_menuroom && !mData->getProjectSettings()->menuroom.empty()){
              mMenuShown = true;
              loadSubRoom(mData->getProjectSettings()->menuroom, NULL, mData->getProjectSettings()->menu_fading);
            }
            else{
              //use internal menu
              Menu* menu = new Menu();
              mRooms.push_front(menu);
              mSubRoomLoaded = true;
              mMenuShown = true;
              menu->setFadeout(mData->getProjectSettings()->menu_fading);
              mAnimator->add(menu, mData->getProjectSettings()->menu_fading, true);
            }
          }
          else{
            unloadRoom(NULL, false, false);
            mMenuShown = false;
          }
        }
      }
    case KEY_BACKSPACE:
      if (!mTextEnter.empty()){
        std::string text = mInterpreter->getVariable(mTextEnter).getString();
        if (!text.empty())
          text.erase(text.size()-1);
        mInterpreter->setVariable(mTextEnter, text);
      }
      break;
    case KEY_RETURN:
      if (!mTextEnter.empty()){
        mInterpreter->resumeBlockingScript();
        mTextEnter.clear();
        mSuspender->resume();
        mFonts->getTextout(-1)->setEnabled(false);
      }
  }
}

void Engine::keyRelease(int key){
  mKeysDown[key] = false;
  mKeysPressed[key] = false;
}

void Engine::keyAscii(char chr){
  if (mTextEnter.empty())
    return;
  std::string text = mInterpreter->getVariable(mTextEnter).getString();
  if (text.size() < mNumCharactersEnter)
    text += chr;
  mInterpreter->setVariable(mTextEnter, text);
}

int Engine::unloadRooms(){
  setFocus("none", NULL);
  if (mCurrentObject){
    mCurrentObject->getScript()->setEvent(EVT_MOUSEOUT);
    delete mDraggingObject;
    mDraggingObject = NULL;
    resetCursor(true, true);
  }
  mCurrentObject = NULL;
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ){
    mRoomsToUnload.push_back(*iter);
    ExecutionContext* script = (*iter)->getScript();
    if (script)
      script->setEvent(EVT_EXIT);
    //if ((*iter)->getScript())
    //  (*iter)->getScript()->resume(); //bulk unload, do unblock scripts
    (*iter)->finishScripts(false);
    iter = mRooms.erase(iter);
  }
  mMainRoomLoaded = false;
  mSubRoomLoaded = false;
  mTaskbar = NULL;
  return mRoomsToUnload.size();
}

std::string Engine::getCharacterClass(const std::string instanceName){
  Rcharacter ch;
  for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
    if (_stricmp(mData->getRoomCharacters()[i].name.c_str(), instanceName.c_str()) == 0){
      return mData->getRoomCharacters()[i].character;
    }
  }
  return "";
}

bool Engine::isKeyDown(int key){
  bool ret = mKeysDown[key];
  mKeysDown[key] = false;
  return ret;
}

bool Engine::isKeyPressed(int key){
  return mKeysPressed[key];
}

void Engine::quit(){
  mExitRequested = true;
}

void Engine::reset(){
  mResetRequested = true;
}

void Engine::renderUnloadingRoom(){
  if (mForceNotToRenderUnloadingRoom){
    return;
  }
  
  RenderableBlitObject mainroom(mData->getProjectSettings()->resolution.x, mData->getProjectSettings()->resolution.y, 0);
  mainroom.setBlendMode(BlitObject::BLEND_PREMULT_ALPHA);
  mainroom.bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  beginRendering();
  if (mRoomsToUnload.size() == 0 && mUnloadedRoom)
    mUnloadedRoom->render();
  else{
    for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
      (*iter)->render();
    }
  }
  endRendering();
  
  mainroom.unbind();
  BlitObject* result = mPostProc->process(&mainroom, 0);
  beginRendering();
  result->render(Vec2i(), Vec2f(1.0f,1.0f), Vec2i());
  endRendering(); 
}

void Engine::remove(Object2D* obj){
  if (mCurrentObject == obj)
    mCurrentObject = NULL;
  if (mClickedObject == obj)
    mClickedObject = NULL;
}

void Engine::enterText(const std::string& variable, int maxcharacters, ExecutionContext* suspensionReason){
  mSuspender = suspensionReason;
  mTextEnter = variable;
  mNumCharactersEnter = maxcharacters;
  mBlinkCursorVisible = true;
  mBlinkTimeAccu = 0;
}

void Engine::restoreRenderDefaults(){
  GL()vertexPointer(2, GL_SHORT, 0, mVerts);
  GL()texCoordPointer(2, GL_SHORT, 0, mVerts);
}

RoomObject* Engine::getMainRoom(){
  if (!mMainRoomLoaded)
    return NULL;
  return mRooms.back();
}

void Engine::removeScript(ExecutionContext* ctx){
  if (ctx == mPendingLoadRoom.reason)
    mPendingLoadRoom.reason = NULL;
}

void Engine::triggerScreenchange(ExecutionContext* loadreason, ScreenChange change, bool unload){
  if (loadreason == NULL || !loadreason->isSkipping()){
    ScreenChangeBase* scb = NULL;
    switch(change){
      case SC_DIRECT:
        break;
      case SC_FADEOUT:{
        scb = new FadeoutScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000);
                      }
                      break;
      case SC_RECTANGLE:{
        scb = new CircleScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000, 4);
                        }
                        break;
      case SC_CIRCLE:{
        scb = new CircleScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000, 64);
                     }
                     break;
      case SC_SHUTTERS:{
        scb = new ShuttersScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000);
                       }
                       break;
      case SC_CLOCK:{
        scb = new ClockScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000, 64);
                    }
                    break;
      case SC_BLEND:{
        scb = new BlendScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 1000);
                    }
                    break;
      case SC_BLEND_SLOW:{
        scb = new BlendScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000);
                         }
                         break;
    }
    if (scb != NULL){
      scb->setUnload(unload);
      Engine::instance()->getAnimator()->add(scb);
    }
  }
}

void Engine::resetCursor(bool resetInstMouse, bool resetDragging){
  if (resetDragging){
    if (mDraggingObject != NULL){
      delete mDraggingObject;
      mDraggingObject = NULL;
    }
  }
  if (resetInstMouse){
    if (mInterpreter->applyPrevState(mCursor)){
      setCommand("none", false);
    }
  }
}
