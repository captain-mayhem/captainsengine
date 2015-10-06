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
#include "ItemObject.h"
#include "GuiRoom.h"
#include "Console.h"
#include <system/allocation.h>

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
  mSaver(NULL), mLoader(), mBlockingSpeaker(NULL), mExitCall(NULL), mSetMouseCall(NULL) {
  mVerts = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
  mVerts->create(VB_POSITION | VB_TEXCOORD, 4);
  mVerts->lockVertexPointer();
  mVerts->setPosition(0, CGE::Vec3f(0, 1, 0));
  mVerts->setPosition(1, CGE::Vec3f(0, 0, 0));
  mVerts->setPosition(2, CGE::Vec3f(1, 1, 0));
  mVerts->setPosition(3, CGE::Vec3f(1, 0, 0));
  mVerts->setTexCoord(0, CGE::Vec2f(0, 1));
  mVerts->setTexCoord(1, CGE::Vec2f(0, 0));
  mVerts->setTexCoord(2, CGE::Vec2f(1, 1));
  mVerts->setTexCoord(3, CGE::Vec2f(1, 0));
  mVerts->unlockVertexPointer();
  mInds = CGE::Engine::instance()->getRenderer()->createIndexBuffer(CGE::IndexBuffer::IB_USHORT, 5);
  mInds->lockIndexPointer();
  mInds->setIndex(0, 2U);
  mInds->setIndex(1, 3U);
  mInds->setIndex(2, 1U);
  mInds->setIndex(3, 0U);
  mInds->setIndex(4, 2U);
  mInds->unlockIndexPointer();
  mAnimator = new Animator();
  mFonts = NULL;
  mInterpreter = NULL;
  mParticleEngine = NULL;
  for (int i = 0; i < 256; ++i){
    mKeysDown[i] = false;
    mKeysPressed[i] = false;
  }
  mConsole = NULL;
#ifndef ENGINE_SINGLE_THREADED
  mLoader.start();
#endif
}

Engine::~Engine(){
#ifndef ENGINE_SINGLE_THREADED
  mLoader.stop();
#endif
  delete mParticleEngine;
  delete mFonts;
  delete mAnimator;
  delete mInterpreter;
  delete mSaver;
  delete mVerts;
  delete mInds;
}

void Engine::setData(AdvDocument* doc){
  mData = doc;
  delete mFonts;
  mInterpreter = new PcdkScript(mData);
  mFonts = new FontRenderer(mData);
  mSaver = new SaveStateProvider(mData);
  mParticleEngine = new ParticleEngine(mData);
  mLoader.setData(doc);
}

void Engine::initGame(exit_callback exit_cb, set_mouse_callback set_mouse_cb){
  TR_USE(ADV_Engine);
  if (!mData){
    TR_ERROR("No data available");
    return;
  }
  if (exit_cb != NULL)
    mExitCall = exit_cb;
  if (set_mouse_cb != NULL)
    mSetMouseCall = set_mouse_cb;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //ExecutionContext* ctx = mInterpreter->parseProgram("playswf (snitt ; 106 ;120 ;427 ; 330)");
  //mInterpreter->execute(ctx, true);
  mMainRoomLoaded = false;
  mSubRoomLoaded = false;
  mFocussedChar = NULL;
  Vec2i res = mData->getProjectSettings()->wm_resolution;
  mWalkGridSize = res.x/32;
  mWalkFields = Vec2i(32,res.y/mWalkGridSize);
  //load cursor
  MouseCursor* cursor = mData->getCursor();
  mCursor = new CursorObject(Vec2i(0,0));
  for (unsigned j = 0; j < cursor->size(); ++j){
    Animation* anim = new Animation((*cursor)[j].frames, (*cursor)[j].fps, (*cursor)[j].highlight*-1, DEPTH_CURSOR, Vec2i(0,0));
    anim->realize();
    mCursor->addAnimation(anim, (*cursor)[j].command-1);
  }
  //load fonts
  int fontcount = 0;
  mFontID = 1;
  if (mFonts->loadFont(0))
    ++fontcount;
  if (mFonts->loadFont(mFontID))
    ++fontcount;
  {
    TR_USE(ADV_Console);
    if (fontcount < 2)
      TR_INFO("Failed to load initial fonts, %i loaded.", fontcount);
    else
      TR_INFO("Initial game fonts loaded (%i).", fontcount);
  }
  mConsole = new Console();
  mConsole->realize();
  mActiveCommand = 0;
  mPrevActiveCommand = 0;
  mCurrentObject = NULL;
  mClickedObject = NULL;
  int transparency = mData->getProjectSettings()->anywhere_transparency*255/100;
  //load taskbar room
  TR_INFO("Loading UI");
  if (mData->getProjectSettings()->show_taskbar && mData->getProjectSettings()->taskroom != ""){
    loadSubRoom(mData->getProjectSettings()->taskroom, NULL, 0);
    TR_DEBUG("waiting for taskbar to load");
    mLoader.waitUntilFinished();
    TR_DEBUG("waiting finished");
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
    mLoader.waitUntilFinished();
    mRooms.front()->setOpacity(255-transparency);
  }
  //load main script
  TR_INFO("Loading start script");
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
  mPendingLoadRoom.room = NULL;
  mPendingLoadRoom.reason = NULL;
  mPendingLoadRoom.screenchange = SC_DIRECT;
  mRenderedMain = new RenderableBlitObject(mData->getProjectSettings()->resolution.x, mData->getProjectSettings()->resolution.y, 0);
  mRenderedMain->realize();
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
  mCoinShown = false;
  //load and execute start script
  Script* startScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->startscript);
  if (startScript){
    TR_USE(ADV_Console);
    TR_INFO("Starting first cutscene...");
    ExecutionContext* initScript = mInterpreter->parseProgram(startScript->text);
    mInterpreter->cutsceneMode(true);
    mInterpreter->executeCutscene(initScript, false);
    TR_INFO("------------------");
  }
  mUI = new GuiRoom();
  TR_INFO("Game initialized successfully");
}

void Engine::exitGame(){
  if (!mInitialized)
    return;
  mLoader.waitUntilFinished();
  if (mPendingLoadRoom.room != NULL){
    mPendingLoadRoom.room->realize();
    delete mPendingLoadRoom.room;
    mPendingLoadRoom.room = NULL;
  }
  mInitialized = false;
  mCurrentObject = NULL;
  mClickedObject = NULL;
  mInterpreter->stop();
  mMainScript->unref();
  mAnimator->clear();
  if (mUnloadedRoom != mConsole)
    delete mUnloadedRoom;
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    mRooms.remove(*iter);
    if (*iter != mConsole)
      delete *iter;
  }
  mRoomsToUnload.clear();
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if (*iter != mConsole)
      delete *iter;
  }
  mRooms.clear();
  clearGui();
  mMainRoomLoaded = false;
  resetCursor(true, false);
  delete mCursor;
  disposeCharacter(mFocussedChar);
  clearCharCache();
  for (std::list<CharacterObject*>::iterator iter = mCharsToUnload.begin(); iter != mCharsToUnload.end(); ++iter){
    delete *iter;
  }
  mCharsToUnload.clear();
  mFonts->unloadFont(mFontID);
  mFonts->unloadFont(0);
  mFonts->clearTextouts();
  mParticleEngine->activate(false, true);
  delete mRenderedMain;
  delete mPostProc;
  delete mDraggingObject;
  mDraggingObject = NULL;
  delete mUI;
  delete mConsole;
}

CGE::Image* Engine::getImage(const std::string& name){
  if (name.empty())
    return NULL;
  if (name[0] == '#'){
    //get special images
    if (name == "#menu_bg"){
      return Menu::getBackground();
    }
    else if (name == "#console_bg"){
      return Console::getBackground();
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

CGE::Texture* Engine::genTexture(const CGE::Image* image, Vec2i& size, Vec2f& scale, const CGE::Image* alphaimage){
  if (!image)
    return 0;
  size.x = image->getWidth();
  size.y = image->getHeight();
  Vec2i pow2(roundToPowerOf2(size.x), roundToPowerOf2(size.y));
  scale.x = ((float)size.x)/pow2.x;
  scale.y = ((float)size.y)/pow2.y;
  CGE::Texture::Format format = CGE::Texture::RGBA;
  GLubyte* buffer;
  bool deleteBuffer = true;
  int numchannels = 4;
  if (image->getNumChannels() == 1){
    format = CGE::Texture::GRAY;
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

  CGE::Texture* tex = CGE::Engine::instance()->getRenderer()->createTexture(pow2.x, pow2.y, format);
  tex->update(0, 0, size.x, size.y, buffer);

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
    Engine::instance()->exitGame();
    Engine::instance()->getSaver()->clear();
    SoundEngine::instance()->reset();
    Engine::instance()->initGame(NULL, NULL);
    mResetRequested = false;
    return;
  }
  if (!mInitialized)
    return;
  //timing
  unsigned interval = (unsigned)(time*mTimeFactor);
  trymtx.lock();
  beginRendering();
  
  //handle realize events
  mLoader.handleResultEvent();

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
      mRoomsToUnload.front()->save(NULL);
    else
      mSaver->allowWrites();
    if (mUnloadedRoom && mUnloadedRoom != mConsole)
      delete mUnloadedRoom;
    if (mForceNotToRenderUnloadingRoom){
      mUnloadedRoom = NULL;
      mForceNotToRenderUnloadingRoom = false;
      if (mRoomsToUnload.front() != mConsole)
        delete mRoomsToUnload.front();
    }
    else
      mUnloadedRoom = mRoomsToUnload.front();
    mRoomsToUnload.pop_front();
    if (mRoomsToUnload.empty() && mPendingLoadRoom.room != NULL){
      //delayed load
      insertRoom(mPendingLoadRoom.room, false, mPendingLoadRoom.reason, mPendingLoadRoom.screenchange, 0);
      if (mPendingLoadRoom.reason){
        mPendingLoadRoom.reason->resume();
        mPendingLoadRoom.reason = NULL;
      }
      mPendingLoadRoom.room = NULL;
      if (!mPendingLoadRoom.focusChar.empty()){
        changeFocus(mPendingLoadRoom.focusChar, NULL);
        mPendingLoadRoom.focusChar = "";
      }
    }
  }
  bool clearChars = true;
  for (std::list<CharacterObject*>::iterator iter = mCharsToUnload.begin(); iter != mCharsToUnload.end(); ++iter){
    if (*iter == NULL)
      continue;
    if ((*iter)->getScript() != NULL && (*iter)->getScript()->isRunning()){
      clearChars = false;
      continue;
    }
    delete *iter;
    *iter = NULL;
  }
  if (clearChars)
    mCharsToUnload.clear();

  //do some scripting
  bool scriptupdate = mInterpreter->willUpdate(interval);
  if (scriptupdate){
    bool hasCursor = (!mInterpreter->isBlockingScriptRunning() || mInterpreter->isTextScene()) && mMouseShown && mMouseEnabled;
    Object2D* obj = getObjectAt(mCursor->getPosition());
    if (hasCursor && obj != NULL){
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
      //always trigger evt_mouse
      ExecutionContext* script = obj->getScript();
      if (script != NULL && !script->containsEvent(EVT_MOUSE)){
        script->setEvent(EVT_MOUSE);
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

    //ui update
    mUI->scriptUpdate();
    
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
      String text = mInterpreter->getVariable(mTextEnter).getString();
      mInterpreter->setVariable(mTextEnter, String((text+"_").c_str()));
    }
  }

   mFonts->prepareTextouts(time);

  if (!mTextEnter.empty() && mBlinkCursorVisible){
    String text = mInterpreter->getVariable(mTextEnter).getString();
    text.erase(text.size()-1);
    mInterpreter->setVariable(mTextEnter, text);
  }

  //build blit queue
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
  int roomdepth = 0;
  RoomObject* mainroom = NULL;
  for (std::list<RoomObject*>::reverse_iterator iter = mRooms.rbegin(); iter != mRooms.rend(); ++iter){
    (*iter)->setDepth(roomdepth);
    roomdepth += DEPTH_ROOM_RANGE;
    if ((*iter) == mTaskbar){
      if (!mShowTaskbar || mInterpreter->isUIHidden())
        continue;
    }
    if ((*iter)->getName() == mData->getProjectSettings()->anywhere_room){
      if (mInterpreter->isUIHidden())
        continue;
    }
    if (mMainRoomLoaded && iter == mRooms.rbegin()){
      mainroom = (*iter);
      mRenderedMain->bind();
      beginRendering();
      mFonts->prepareBlit(interval, mainroom, true);
      //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      rend->clear(COLORBUFFER | ZBUFFER);
      rend->pushMatrix();
      rend->resetModelView();
      rend->translate(0.0f, (float)Engine::instance()->getResolution().y, 0.0f);
      rend->scale(1.0f,-1.0f,1.0f);
    }
    (*iter)->render();
    if (mMainRoomLoaded && iter == mRooms.rbegin()){
      if (mFocussedChar && mFocussedChar->getRoom() == (*iter)->getName())
        mFocussedChar->render();
      if (mParticleEngine->getDepth() != DEPTH_PARTICLES_TOP) //render the particles now if they belong to the main room
        mParticleEngine->update(interval);
      endRendering();
      rend->popMatrix();
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
  mUI->render();

  //command handling
  Vec2i res = mData->getProjectSettings()->resolution;
  std::string text;
  if (mFocussedChar && !mInterpreter->isBlockingScriptRunning()){
    int cmdidx;
    if ((mUseObjectName.empty() && mGiveObjectName.empty()) || mPrevActiveCommand == 0)
      cmdidx = mActiveCommand;
    else
      cmdidx = mPrevActiveCommand;
    if (cmdidx == mData->getProjectSettings()->commands["loading"]) //don't display loading state
      cmdidx = 0;
    int langcmdidx = cmdidx;
    if (langcmdidx > 0){
      //the command section is not neccessarily ordered, so search the right index
      langcmdidx = mData->getLanguageIndex("origin", Language::COMMANDS, mData->getProjectSettings()->pretty_commands[cmdidx]);
    }
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
  //calculate depth
  int depthoffset = 0;
  if (isSubRoom){
    depthoffset = (int)mRooms.size()*1000;
    if (!mMainRoomLoaded)
      depthoffset += 1000;
  }
  else{
    mCursor->showLoading(true);
  }

  //trigger async loader
  TR_DETAIL("loadRoom called for %s", name.c_str());
  if (loadreason)
    loadreason->suspend(0, NULL);
  mLoader.loadRoom(name, isSubRoom, loadreason, change, fading, depthoffset);

  return true;
}

void Engine::insertRoom(RoomObject* roomobj, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading){
  if (mMainRoomLoaded && !isSubRoom){
    TR_USE(ADV_Engine);
    TR_INFO("delay activation of room %s", roomobj->getName().c_str());
    unloadRoom(mRooms.back(), true, false, loadreason);
    if (mPendingLoadRoom.reason != NULL){
      mPendingLoadRoom.reason->resume();
    }
    mPendingLoadRoom.room = roomobj;
    mPendingLoadRoom.reason = loadreason;
    mPendingLoadRoom.screenchange = change;
    mPendingLoadRoom.focusChar = "";
    return;
  }
  else if (loadreason)
    loadreason->resume();

  roomobj->realize();
  if (mData->getProjectSettings()->coinActivated && roomobj->getName() == mData->getProjectSettings()->coinRoom){
    roomobj->setPosition(getCursorPos()-mData->getProjectSettings()->coinCenter);
  }
  TR_USE(ADV_Console);
  TR_INFO("Room \"%s\" loaded.", roomobj->getName().c_str());

  if (isSubRoom){
    mRooms.push_front(roomobj);
    if (mInitialized)
      mSubRoomLoaded = true;
  }
  else{
    mRooms.push_back(roomobj);
    mMainRoomLoaded = true;
  }
  if (_stricmp(roomobj->getName().c_str(), mData->getProjectSettings()->taskroom.c_str()) == 0)
    mTaskbar = roomobj;
  if (mFocussedChar && mFocussedChar->getRoom() == roomobj->getName()){
    mFocussedChar->setScale(roomobj->getDepthScale(mFocussedChar->getPosition()));
    mFocussedChar->setScrollOffset(roomobj->getScrollOffset());
    mFocussedChar->setDepth((int)(mFocussedChar->getPosition().y/Engine::instance()->getWalkGridSize(true)));
  }
  //animation stuff
  if (!isSubRoom)
    triggerScreenchange(loadreason, change, false);
  else if (fading != 0){
    roomobj->setFadeout(fading);
    mAnimator->add(roomobj, fading, true);
  }
}

void Engine::unloadRoom(RoomObject* room, bool mainroom, bool immediately, ExecutionContext* reason){
  if (mRooms.empty() || (mainroom && !mMainRoomLoaded))
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
  room->unbindScript(reason); //prevent the script that triggered the unloading from being skipped or aborted
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
        mFonts->disableBoundTextouts(room);
        mAnimator->add(room, room->getFadeout(), false);
      }
    }
  }
}

void Engine::beginRendering(){
  mBlitQueues.push_back(std::list<BaseBlitObject*>());
}

void Engine::endRendering(bool inverse){
  restoreRenderDefaults();
  if (inverse){
    for (std::list<BaseBlitObject*>::reverse_iterator iter = mBlitQueues.back().rbegin(); iter != mBlitQueues.back().rend(); ++iter){ 
      (*iter)->blit();
    }
  }
  else{
    for (std::list<BaseBlitObject*>::iterator iter = mBlitQueues.back().begin(); iter != mBlitQueues.back().end(); ++iter){ 
      (*iter)->blit();
    }
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
      if (!mUseObjectName.empty()){
        script->setUseObjectName(mUseObjectName);
        script->setEvent(EVT_LINK);
      }
      else if (!mGiveObjectName.empty()){
        script->setGiveObjectName(mGiveObjectName);
        script->setEvent(EVT_GIVE_LINK);
      }
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
    if (curCmd != mActiveCommand && mLinkObjectInfo.empty() ||
      curCmd == mActiveCommand && !mLinkObjectInfo.empty()){
      mPrevActiveCommand = mActiveCommand;
      mActiveCommand = curCmd;
      mUseObjectName = "";
      mGiveObjectName = "";
      mLinkObjectInfo = "";
      resetCursor(true, true);
    }
  }
  if (mCoinShown){
    RoomObject* cr = getRoom(mData->getProjectSettings()->coinRoom);
    if (cr){
      unloadRoom(cr, false, false, NULL);
      mCoinShown = false;
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
  if (mBlockingSpeaker){
    CharacterObject* speaker = mBlockingSpeaker;
    mFonts->removeText(speaker, false);
    SoundEngine::instance()->removeSpeaker(speaker);
    return;
  }
  bool leftClickRequired;
  int cmd = mCursor->getNextCommand(leftClickRequired, pos);
  leftClickRequired = leftClickRequired && !(mData->getProjectSettings()->coinActivated && mData->getProjectSettings()->coinAutoPopup);
  mActiveCommand = cmd;
  mUseObjectName = "";
  mGiveObjectName = "";
  mLinkObjectInfo = "";
  if (leftClickRequired)
    leftClick(pos);
  Object2D* obj = getObjectAt(pos);
  if (obj != NULL){
    if (mData->getProjectSettings()->coinActivated && mData->getProjectSettings()->coinAutoPopup)
      popupCoinMenu(NULL);
    else{
      ExecutionContext* script = obj->getScript();
      if (script != NULL)
        script->setEvent(EVT_RIGHTCLICK);
    }
  }
  if (mCoinShown){
    RoomObject* cr = getRoom(mData->getProjectSettings()->coinRoom);
    if (cr){
      unloadRoom(cr, false, false, NULL);
      mCoinShown = false;
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
      SaveStateProvider::SaveRoom* sr = mSaver->getRoom(mFocussedChar->getRoom());
      mFocussedChar->save(sr);
      deletionChar = mFocussedChar;
    }
  }
  if (charname == "none"){
    if (mFocussedChar)
      mLastFocussedChar = mFocussedChar->getName();
    mFocussedChar = NULL;
    disposeCharacter(deletionChar);
    return true;
  }
  if (charname == "last"){
    charname = mLastFocussedChar;
  }
  CharacterObject* res = extractCharacter(charname);
  if (res){
    mFocussedChar = res;
    disposeCharacter(deletionChar);
    return true;
  }
  //load character async with room loading
  mCursor->showLoading(true);
  mLoader.setFocus(charname, reason);
  mFocussedChar = NULL;
  disposeCharacter(deletionChar);
  return false;
}

void Engine::changeFocus(std::string charname, ExecutionContext* reason){
  if (mPendingLoadRoom.room != NULL){
    mPendingLoadRoom.focusChar = charname;
    return;
  }
  if (reason)
    reason->resume();
  setFocus(charname, reason);
}

bool Engine::aStarSearch(CharacterObject* chr, const Vec2i& from, const Vec2i& to, std::list<Vec2i>& path){
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
        case 2:
          y.pos.y -= 1;
          if (y.pos.y < 0)
            continue;
          break;
        case 3:
          y.pos.y += 1;
          if (y.pos.y >= mData->getProjectSettings()->resolution.y/mWalkGridSize*4)
            continue;
          break;
        case 0:
          y.pos.x -= 1;
          if (y.pos.x < 0)
            continue;
          break;
        case 1:
          y.pos.x += 1;
          if (y.pos.x >= mData->getProjectSettings()->resolution.x/mWalkGridSize*6)
            continue;
          break;
      }
      if (closedset.find(y) != closedset.end())
        continue;
      if (!mRooms.back()->isWalkable(chr, y.pos))
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
  Object2D* ret = mUI->getObjectAt(pos);
  if (ret != NULL)
    return ret;
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
  Object2D* ret = mUI->getObject(name);
  return ret;
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
      mSaver->removeCharacter(srm, res->getName());
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
  float walkgridsize = chr->getWalkGridSize();
  Vec2f oldwmpos = ((Vec2f)chr->getPosition())/walkgridsize;
  Vec2f newwmpos = ((Vec2f)pos)/walkgridsize;
  std::list<Vec2i> path;
  bool couldReach = aStarSearch(chr, oldwmpos, newwmpos, path); 
  //remove first entry as we are already on that field
  path.pop_front();
  for (std::list<Vec2i>::iterator iter = path.begin(); iter != path.end(); ++iter){
    *iter = *iter*walkgridsize+Vec2f(walkgridsize/2,walkgridsize/2);
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

ItemObject* Engine::createItem(const std::string& name, int count){
  Item* it = mData->getItem(name);
  if (it == NULL){
    TR_USE(ADV_Engine);
    TR_BREAK("Item %s not found", name.c_str());
    return NULL;
  }
  int state = mInterpreter->getItemState(name);
  ItemObject* object = new ItemObject(state, Vec2i(), Vec2i(50,50), it->name);
  object->setCount(count);
  int depth = DEPTH_ITEM;
  for (unsigned j = 0; j < it->states.size(); ++j){
    Animation* anim = new Animation(it->states[j].frames, it->states[j].fps, Vec2i(), depth, Vec2i(50,50));
    object->addAnimation(anim);
  }
  //check for object scripts
  Script* script = mData->getScript(Script::ITEM,it->name);
  if (script){
    ExecutionContext* scr = mInterpreter->parseProgram(script->text);
    object->setScript(scr);
  }
  return object;
}

void Engine::setUseObject(const std::string& object, const std::string& objectInfo){
  mLinkObjectInfo = objectInfo;
  mUseObjectName = object;
  handleDragging(object);
}

void Engine::setGiveObject(const std::string& object, const std::string& objectInfo){
  mLinkObjectInfo = objectInfo;
  mGiveObjectName = object;
  handleDragging(object);
}

void Engine::handleDragging(const std::string& object){
  if (mData->getProjectSettings()->draw_dragged_items){
    delete mDraggingObject;
    if (object.empty() || mClickedObject == NULL)
      mDraggingObject = NULL;
    else{
      mDraggingObject = createItem(mClickedObject->getName(), 1);
      if (mDraggingObject){
        mDraggingObject->realize();
        mDraggingObject->setDepth(DEPTH_CURSOR-1);
      }
    }
  }
}

ExecutionContext* Engine::loadScript(Script::Type type, const std::string& name){
  Script* scr = mData->getScript(type, name);
  ExecutionContext* ctx = mInterpreter->parseProgram(scr->text);
  return ctx;
}

void Engine::addUIElement(Object2D* elem, int offset){
  int fading = Engine::instance()->getSettings()->textscene_fading;
  if (fading > 0 && mUI->getFadeout() == 0){
    mUI->setFadeout(fading);
    Engine::instance()->getAnimator()->add(mUI, fading, true);
  }
  mUI->addUIElement(elem, offset);
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

EngineEvent Engine::getActiveCommandAsEvent(){
  if (!mUseObjectName.empty())
    return EVT_LINK;
  else if (!mGiveObjectName.empty())
    return EVT_GIVE_LINK;
  return (EngineEvent)mActiveCommand;
}

void Engine::clearGui(){
  if (mUI->clear()){
    int fading = Engine::instance()->getSettings()->textscene_fading;
    Engine::instance()->getAnimator()->add(mUI, fading, false);
  }
}

RoomObject* Engine::getContainingRoom(Object2D* object){
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if ((*iter)->containsObject(object))
      return *iter;
  }
  return NULL;
}

CharacterObject* Engine::loadCharacter(const std::string& instanceName, const std::string& className, ExecutionContext* loadreason){
  TR_USE(ADV_Engine);
  SaveStateProvider::CharSaveObject* obj = NULL;
  std::string room;
  std::string realName;
  if (mFocussedChar && _stricmp(mFocussedChar->getName().c_str(), instanceName.c_str()) == 0)
    return NULL;
  if (!obj){
    obj = mSaver->findCharacter(instanceName, room, realName);
    if (obj == NULL){
      TR_WARN("Character %s not found or currently beamed around", instanceName.c_str());
      return NULL;
    }
  }
  Character* chbase = mData->getCharacter(className);
  if (chbase == NULL)
    return NULL;
  String key = instanceName;
  key = key.toLower();
  bool fromCache = chbase->memresistent && mCharCache.find(key) != mCharCache.end();
  CharacterObject* character;
  if (fromCache){
    character = mCharCache[key];
    character->Object2D::setPosition(obj->base.position);
    character->setState(obj->base.state);
    if (obj->mirrored)
      character->setLookDir(LEFT);
  }
  else{
    character = new CharacterObject(chbase, obj->base.state, obj->mirrored, obj->base.position, realName);
    if (chbase->memresistent)
      mCharCache[key] = character;
  }
  character->setLightingColor(obj->base.lighting);
  character->setFontID(obj->fontid);
  //mFonts->loadFont(obj->fontid);
  character->setUserScale(obj->scale);
  character->setTextColor(chbase->textcolor);
  character->setRoom(room);
  character->setNoZooming(obj->nozooming, true);
  if (!fromCache && !obj->walksound.empty()){
    SoundPlayer* plyr = SoundEngine::instance()->getSound(obj->walksound, SoundEngine::PLAYER_CREATE_ALWAYS | SoundEngine::PLAYER_UNMANAGED | SoundEngine::PLAYER_UNREALIZED);
    character->setWalkSound(plyr);
  }
  if (!obj->linkedObject.empty()){
    Object2D* link = getObject(obj->linkedObject, false);
    character->setLinkObject(link);
  }
  //RoomObject* ro = Engine::instance()->getRoom(room);
  //if (ro)
  //  character->setScale(ro->getDepthScale(obj->base.position));
  if (!fromCache){
    for (unsigned j = 0; j < chbase->states.size(); ++j){
      int depth = (int)((obj->base.position.y+chbase->states[j].basepoint.y)/Engine::instance()->getWalkGridSize(true));
			Animation* anim = new Animation(chbase->states[j].frames, chbase->states[j].fps, depth, chbase->states[j].size);
      character->addAnimation(anim);
      character->addBasepoint(chbase->states[j].basepoint, chbase->states[j].size);
    }
    Script* script = mData->getScript(Script::CHARACTER,instanceName);
    if (script){
      ExecutionContext* scr = mInterpreter->parseProgram(script->text);
      if (scr){
        character->setScript(scr);
        //mInterpreter->execute(scr, false);
      }
    }
  }
  character->getInventory()->load(obj->inventory);
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
      mSaver->load(SaveStateProvider::saveSlotToPath(0), false);
      break;
    case KEY_ESCAPE:
      {
        ExecutionContext* ctx = mInterpreter->getCutscene();
        if (ctx){
          if (mBlockingSpeaker){
            CharacterObject* speaker = mBlockingSpeaker;
            mFonts->removeText(speaker, false);
            SoundEngine::instance()->removeSpeaker(speaker);
          }
          ctx->setSkip();
        }
        else if (mConsole->isShown()){
          mConsole->show(false);
          unloadRoom(mConsole, false, false, NULL);
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
              Menu* menu = new Menu(mData->hasUnifiedFonts() ? 1 : 0);
              menu->realize();
              mRooms.push_front(menu);
              mSubRoomLoaded = true;
              mMenuShown = true;
              if (mData->getProjectSettings()->menu_fading > 0){
                menu->setFadeout(mData->getProjectSettings()->menu_fading);
                mAnimator->add(menu, mData->getProjectSettings()->menu_fading, true);
              }
            }
          }
          else{
            unloadRoom(NULL, false, false, NULL);
            mMenuShown = false;
          }
        }
      }
    case KEY_BACKSPACE:
      if (!mTextEnter.empty()){
        String text = mInterpreter->getVariable(mTextEnter).getString();
        if (!text.empty())
          text.erase(text.size()-1);
        mInterpreter->setVariable(mTextEnter, text);
      }
      break;
    case KEY_RETURN:
      finishTextInput(true);
      break;
    case KEY_CIRCUMFLEX:
      if (!mConsole->isShown()){
        mRooms.push_front(mConsole);
        mConsole->show(true);
      }
      else{
        mConsole->show(false);
        unloadRoom(mConsole, false, false, NULL);
      }
      break;
    case KEY_UP:
      if (mConsole->isShown()){
        mConsole->historyUp();
      }
      break;
    case KEY_DOWN:
      if (mConsole->isShown()){
        mConsole->historyDown();
      }
      break;
    case KEY_PAGEUP:
      if (mConsole->isShown()){
        mConsole->scrollUp();
      }
      break;
    case KEY_PAGEDOWN:
      if (mConsole->isShown()){
        mConsole->scrollDown();
      }
      break;
  }
}

void Engine::keyRelease(int key){
  mKeysDown[key] = false;
  mKeysPressed[key] = false;
}

void Engine::keyAscii(char chr){
  if (chr == '^') //remove this special character (open console)
    return;
  if (mTextEnter.empty())
    return;
  String text = mInterpreter->getVariable(mTextEnter).getString();
  if (text.size() < mNumCharactersEnter)
    text += chr;
  mInterpreter->setVariable(mTextEnter, text);
}

void Engine::finishTextInput(bool commit){
  if (!mTextEnter.empty()){
    if (mConsole->isShown() && commit){
      StackData val = mInterpreter->getVariable("!consoleInput");
      mInterpreter->setVariable("!consoleInput", "");
      Engine::instance()->getConsole()->input(val.getString().c_str());
    }
    mInterpreter->resumeBlockingScript();
    mTextEnter.clear();
    mSuspender->resume();
    mFonts->getTextout(-1)->setEnabled(false);
  }
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
    (*iter)->setFadeout(0); //skip fading of subrooms
    mRoomsToUnload.push_back(*iter);
    ExecutionContext* script = (*iter)->getScript();
    if (script)
      script->setEvent(EVT_EXIT);
    //bulk unload, do unblock scripts
    (*iter)->finishScripts(false);
    iter = mRooms.erase(iter);
  }
  mMainRoomLoaded = false;
  mSubRoomLoaded = false;
  mTaskbar = NULL;
  return (int)mRoomsToUnload.size();
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
  mainroom.realize();
  mainroom.setBlendMode(BlitObject::BLEND_PREMULT_ALPHA);
  mainroom.bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  beginRendering();
  //a workaround to make screen shots correct that are taken too late
  bool oldMainLoaded = mMainRoomLoaded;
  mMainRoomLoaded = true;
  bool inserted = false;
  if (mRoomsToUnload.size() == 0 && mUnloadedRoom){
    mRooms.push_back(mUnloadedRoom);
    inserted = true;
    mUnloadedRoom->render();
  }
  else{
    if (!mRoomsToUnload.empty()){
      inserted = true;
      mRooms.push_back(mRoomsToUnload.back());
    }
    for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
      (*iter)->render();
    }
  }
  mMainRoomLoaded = oldMainLoaded;
  if (inserted)
    mRooms.pop_back();
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

void Engine::enterText(const String& variable, int maxcharacters, ExecutionContext* suspensionReason){
  mSuspender = suspensionReason;
  mTextEnter = variable;
  mNumCharactersEnter = maxcharacters;
  mBlinkCursorVisible = true;
  mBlinkTimeAccu = 0;
}

void Engine::restoreRenderDefaults(){
  mVerts->activate();
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
    else
      mCursor->showLoading(false);
  }
  else{
    mCursor->showLoading(false);
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

void Engine::insertCharacter(CharacterObject* obj, std::string roomname, Vec2i pos, LookDir dir){
  if (!obj)
    return;
  obj->abortClick();   
  obj->setTalking(false);
  obj->setWalking(false);
  if (dir != UNSPECIFIED)
    obj->setLookDir(dir);
  if (roomname.empty())
    roomname = obj->getRoom();
  RoomObject* room = Engine::instance()->getRoom(roomname);
  SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(roomname);
  if (room){
    obj->setRoom(room->getName());
    obj->setPosition(Vec2i(pos*room->getWalkGridSize()+Vec2f(room->getWalkGridSize()/2, room->getWalkGridSize()/2)));
    obj->setScale(room->getDepthScale(obj->getPosition()));
    obj->setDepth((int)(obj->getPosition().y/Engine::instance()->getWalkGridSize(true)));
    Engine::instance()->getSaver()->removeCharacter(sr, obj->getName());
    room->addObject(obj);
  }
  else{
    obj->setRoom(roomname);
    Engine::instance()->getSaver()->removeCharacter(sr, obj->getName());
    obj->setPosition(Vec2i(pos*sr->getWalkGridSize()+Vec2f(sr->getWalkGridSize()/2, sr->getWalkGridSize()/2)));
    obj->save(sr);
    disposeCharacter(obj);
  }
}

void Engine::enableTextScene(bool doit){
  mUI->setAllowAdd(doit);
}

void Engine::setBlockingSpeaker(CharacterObject* chr){
  if (chr == NULL){
    --mBlockingSpeakerCount;
    if (mBlockingSpeakerCount == 0)
      mBlockingSpeaker = NULL;
  }
  else{
    if (mBlockingSpeaker == chr)
      ++mBlockingSpeakerCount;
    else{
      mBlockingSpeaker = chr;
      mBlockingSpeakerCount = 1;
    }
  }
}

void Engine::setMousePosition(int x, int y){
  if (mSetMouseCall)
    mSetMouseCall(x, y);
}

void Engine::disposeCharacter(CharacterObject* character){
  TR_USE(ADV_Engine);
  if (character == NULL)
    return;
  if (!character->getClass()->memresistent){
    mCharsToUnload.push_back(character);
  }
}

void Engine::clearCharCache(){
  for (std::map<String, CharacterObject*>::iterator iter = mCharCache.begin(); iter != mCharCache.end(); ++iter){
    delete iter->second;
  }
  mCharCache.clear();
}

void Engine::popupCoinMenu(ExecutionContext* loadreason){
  if (mData->getProjectSettings()->coinActivated){
    loadSubRoom(mData->getProjectSettings()->coinRoom, loadreason, mData->getProjectSettings()->coinFading);
    mCoinShown = true;
  }
}

void Engine::drawQuad(){
  mVerts->draw(CGE::VB_Tristrip, NULL);
}

void Engine::drawQuadLines(){
  mVerts->draw(CGE::VB_Linestrip, mInds);
}
