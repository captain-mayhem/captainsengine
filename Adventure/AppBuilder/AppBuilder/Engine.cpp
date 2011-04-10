#include "Engine.h"

#include <functional>
#include <numeric>
#include <queue>

#include <input/keyboard.h>

#include "AdvDoc.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "Screenchange.h"

#ifndef WIN32
void DebugBreak(){
__builtin_trap();
}

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

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL), mInitialized(false), mExitRequested(false), mResetRequested(false){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
  mAnimator = new Animator();
  mFonts = NULL;
  mInterpreter = NULL;
  for (int i = 0; i < 256; ++i){
    mKeysDown[i] = false;
    mKeysPressed[i] = false;
  }
}

Engine::~Engine(){
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
}

void Engine::initGame(exit_callback exit_cb){
  if (!mData)
    return;
  if (exit_cb != NULL)
    mExitCall = exit_cb;
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
  //load taskbar room
  if (mData->getProjectSettings()->taskroom != ""){
    loadRoom(mData->getProjectSettings()->taskroom, true, NULL);
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
    loadRoom(mData->getProjectSettings()->anywhere_room, true, NULL);
  }
  //load main script
  Script* mainScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->mainscript);
  if (mainScript){
    mMainScript = mInterpreter->parseProgram(mainScript->text);
    mInterpreter->execute(mMainScript, false);
  }
  else
    mMainScript = NULL;
  //load and execute start script
  Script* startScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->startscript);
  if (startScript){
    ExecutionContext* initScript = mInterpreter->parseProgram(startScript->text);
    mInterpreter->executeCutscene(initScript, false);
  }
  mUseObjectName = "";
  mGiveObjectName = "";
  mInitialized = true;
  mShowTaskbar = true;
  mScreenChange = mData->getProjectSettings()->screenchange;
  mTextEnabled = true;
}

void Engine::exitGame(){
  if (!mInitialized)
    return;
  mInitialized = false;
  mInterpreter->stop();
  delete mMainScript;
  mAnimator->clear();
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    delete *iter;
  }
  mRoomsToUnload.clear();
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    delete *iter;
  }
  mRooms.clear();
  clearGui();
  mMainRoomLoaded = false;
  delete mCursor;
  delete mFocussedChar;
  mFonts->unloadFont(mFontID);
  mFonts->unloadFont(0);
}

CGE::Image* Engine::getImage(const std::string& name){
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
  unsigned totalsize = size.x*size.y;
  unsigned char* rgb = image->getData();
  //unsigned char* alpha = image.GetAlpha();
  GLubyte* buffer = new GLubyte[totalsize*4];
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
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  char* init = new char[pow2.x*pow2.y*4];
  memset(init, 0, pow2.x*pow2.y*4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, init);
  delete [] init;
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  /*glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_R,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);*/
  delete buffer;
  return tex;
}

void Engine::render(unsigned time){
  if (mExitRequested){
    mExitCall();
    return;
  }
  if (mResetRequested){
    Engine::instance()->setFocus("none", NULL);
    Engine::instance()->getSaver()->clear();
    Engine::instance()->exitGame();
    Engine::instance()->initGame(NULL);
    mResetRequested = false;
    return;
  }
  if (!mInitialized)
    return;
  //timing
  unsigned interval = time;
  
  //unload rooms
  while (!mRoomsToUnload.empty()){
    //do not unload when a script is in progress
    if (mRoomsToUnload.front()->isScriptRunning())
      break;
    ExecutionContext* ctx = mRoomsToUnload.front()->getScript();
    if (ctx){
      ctx->setEvent(EVT_EXIT);
      mInterpreter->executeImmediately(ctx);
    }
    if (mSaver->isWriteAllowed())
      mRoomsToUnload.front()->save();
    delete mRoomsToUnload.front();
    mRoomsToUnload.pop_front();
  }
  mSaver->allowWrites(true);

  //do some scripting
  Object2D* obj = getObjectAt(mCursor->getPosition());
  if (obj != NULL){
    ExecutionContext* script = obj->getScript();
    if (script != NULL){
      script->setEvent(EVT_MOUSE);
    }
  }
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    ExecutionContext* script = (*iter)->getScript();
    if (script != NULL){
      script->setEvent(EVT_LOOP1);
      script->setEvent(EVT_LOOP2);
    }
  }
  if (mRooms.size() > 0 && mFocussedChar && !mInterpreter->isBlockingScriptRunning()){ //walkmap
    Vec2i pos = mFocussedChar->getPosition()/mWalkGridSize;
    mRooms.back()->walkTo(pos);
  }

  //ui update
  for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
    mInterpreter->executeImmediately((*iter)->getScript());
  }
  clearGui();

  mInterpreter->update(interval);

  //can't all
  if (mFocussedChar){
    EngineEvent evt = mFocussedChar->getScript()->getCommandEvent();
    if (evt >= EVT_USER_MIRROR_BEGIN && evt <= EVT_USER_MIRROR_END){
      mFocussedChar->getScript()->resetEvent(evt);
      mFocussedChar->getScript()->setEvent(EVT_CANT_ALL);
    }
  }

  //scrolling
  if (mRooms.size() > 0 && mFocussedChar){
    mScrollOffset = mData->getProjectSettings()->resolution/2-
      (mFocussedChar->getPosition()-Vec2i(0,mFocussedChar->getSize().y/2));
    mRooms.back()->setScrollOffset(mScrollOffset); //this function limits the scrolling
    mScrollOffset = mRooms.back()->getScrollOffset();
    mFocussedChar->setScrollOffset(mScrollOffset);
  }

  //some animation stuff
  mAnimator->update(interval);
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    (*iter)->update(interval);
  }
  if (mFocussedChar && mFocussedChar->getAnimation())
    mFocussedChar->getAnimation()->update(interval);

  //build blit queue
  for (std::list<RoomObject*>::reverse_iterator iter = mRooms.rbegin(); iter != mRooms.rend(); ++iter){
    if ((*iter) == mTaskbar){
      if (!mShowTaskbar)
        continue;
    }
    (*iter)->render();
    if (mInterpreter->isBlockingScriptRunning())
      break;
  }
  if (mFocussedChar && mFocussedChar->getRoom() == mRooms.back()->getName())
    mFocussedChar->render();
  mCursor->render();
  for (std::list<Object2D*>::iterator iter = mUI.begin(); iter != mUI.end(); ++iter){
    (*iter)->render();
  }

  //command handling
  Vec2i res = mData->getProjectSettings()->resolution;
  std::string text;
  if ((mUseObjectName.empty() && mGiveObjectName.empty()) || mPrevActiveCommand == 0)
    text = mData->getProjectSettings()->pretty_commands[mActiveCommand];
  else
    text = mData->getProjectSettings()->pretty_commands[mPrevActiveCommand];
  if (!mUseObjectName.empty()){
    text += " "+mLinkObjectInfo;
    text += " "+mData->getProjectSettings()->linktext;
  }
  if (!mGiveObjectName.empty()){
    text += " "+mLinkObjectInfo;
    text += " "+mData->getProjectSettings()->givelink;
  }
  if (!mObjectInfo.empty()){
    text += " "+mObjectInfo;
    mObjectInfo.clear();
  }
  //elevate the action line
  if (mTaskbar && !mInterpreter->isBlockingScriptRunning() && mShowTaskbar){
    res.y = mTaskbar->getPosition().y;
  }
  std::vector<Vec2i> breakinfo;
  Vec2i offset = mFonts->getTextExtent(text, 1, breakinfo);
  if (!mInterpreter->isBlockingScriptRunning() && mFocussedChar != NULL)
    mFonts->render(res.x/2-offset.x/2, res.y-offset.y, text, DEPTH_GAME_FONT, 1, breakinfo);

  mFonts->prepareBlit(interval);

  //render the stuff
  GL()vertexPointer(2, GL_SHORT, 0, mVerts);
  GL()texCoordPointer(2, GL_SHORT, 0, mVerts);
  for (std::list<BaseBlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){ 
    (*iter)->blit();
  }
  mBlitQueue.clear();
}

bool Engine::loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason){
  if (!mData)
    return false;
  //already loaded //TODO subrooms
  if (mMainRoomLoaded && _stricmp(mRooms.back()->getName().c_str(), name.c_str()) == 0)
    return true;
  Room* room = mData->getRoom(name);
  SaveStateProvider::SaveRoom* save = mSaver->getRoom(room->name);
  if (!room || !save)
    return false;
  if (mMainRoomLoaded && !isSubRoom){
    unloadRoom(mRooms.back(), true);
  }
  int depthoffset = 0;
  if (isSubRoom){
    depthoffset = (int)mRooms.size()*1000;
    if (!mMainRoomLoaded)
      depthoffset += 1000;
  }
  Room* rm = mData->getRoom(name);
  RoomObject* roomobj = new RoomObject(save->base.state, save->base.position, room->size, room->name, rm->depthmap);
  roomobj->setParallaxBackground(room->parallaxbackground, depthoffset-2);
  //anywhere room is not allowed to have background
  if (_stricmp(room->name.c_str(), mData->getProjectSettings()->anywhere_room.c_str()) != 0)
    roomobj->setBackground(room->background, depthoffset-1);
  roomobj->setLightingColor(save->base.lighting);
  roomobj->setWalkmap(room->walkmap);
  roomobj->setScrollOffset(save->scrolloffset);
  //check for walkmap scripts
  std::vector<std::pair<Vec2i,Script*> > wmscripts = mData->getWMScripts(room->name);
  for (unsigned i = 0; i < wmscripts.size(); ++i){
    ExecutionContext* scr = mInterpreter->parseProgram(wmscripts[i].second->text);
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
      depth = 0;
    else if (room->objects[i].layer == 1)
      depth = room->objects[i].wm_depth-1;
    else
      depth = 990;
    //object->setScale(roomobj->getDepthScale(saveobj->position));
    for (unsigned j = 0; j < o->states.size(); ++j){
      Animation* anim = new Animation(o->states[j].frames, o->states[j].fps, depth+depthoffset);
      object->addAnimation(anim);
    }
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
  //load room script
  Script* script = mData->getScript(Script::ROOM,room->name);
  if (script){
    ExecutionContext* scr = mInterpreter->parseProgram(script->text);
    if (scr != NULL){
      roomobj->setScript(scr);
      mInterpreter->execute(scr, false);
      scr->setEvent(EVT_ENTER);
      mInterpreter->executeImmediately(scr);
    }
  }
  //inventory display
  if (room->invsize != Vec2i()){
    InventoryDisplay* disp = new InventoryDisplay(room->invpos, room->invsize, room->invscale);
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
  //animation stuff
  if (loadreason == NULL || !loadreason->isSkipping()){
    switch(Engine::instance()->getScreenChange()){
      case SC_DIRECT:
        break;
      case SC_FADEOUT:{
        FadeoutScreenChange* fsc = new FadeoutScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000);
        Engine::instance()->getAnimator()->add(fsc);
        }
        break;
      case SC_RECTANGLE:
        DebugBreak();
        break;
      case SC_CIRCLE:{
        CircleScreenChange* csc = new CircleScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000);
        Engine::instance()->getAnimator()->add(csc);
        }
        break;
      case SC_SHUTTERS:
        DebugBreak();
        break;
      case SC_CLOCK:
        DebugBreak();
        break;
      case SC_BLEND:{
        BlendScreenChange* bsc = new BlendScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 1000);
        Engine::instance()->getAnimator()->add(bsc);
        }
        break;
      case SC_BLEND_SLOW:{
        BlendScreenChange* bsc = new BlendScreenChange(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, DEPTH_SCREENCHANGE, 2000);
        Engine::instance()->getAnimator()->add(bsc);
        }
        break;
    }
  }
  return true;
}

void Engine::unloadRoom(RoomObject* room, bool mainroom){
  if (room == NULL){
    if (mainroom){
      room = mRooms.back();
    }
    else
      room = mRooms.front();
  }
  mRoomsToUnload.push_back(room);
  room->skipScripts();
  //room->save();
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

void Engine::insertToBlit(BaseBlitObject* obj){
  bool placenotfound = true;
  for (std::list<BaseBlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){
    if (obj->getDepth() <= (*iter)->getDepth()){
      mBlitQueue.insert(iter, obj);
      placenotfound = false;
      break;
    }
  }
  if (placenotfound)
    mBlitQueue.push_back(obj);
}

void Engine::setCursorPos(Vec2i pos){
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
}

Vec2i Engine::getCursorPos(){
  return mCursor->getPosition();
}

void Engine::leftClick(const Vec2i& pos){
  ExecutionContext* script = NULL;
  Object2D* obj = getObjectAt(pos);
  if (obj != NULL){
    script = obj->getScript();
    if (script != NULL){
      script->setEvent(EVT_CLICK);
      if (!mUseObjectName.empty())
        script->setEvent(EVT_LINK);
      else if (!mGiveObjectName.empty())
        script->setEvent(EVT_GIVE_LINK);
      else
        script->setEvent((EngineEvent)mActiveCommand);
      script->setStepEndHandler(PcdkScript::clickEndHandler);
    }
    else if (mFocussedChar && !mSubRoomLoaded){
      Engine::instance()->walkTo(mFocussedChar, pos-mScrollOffset, UNSPECIFIED);
    }
  }
  else if (mFocussedChar && !mSubRoomLoaded && !mInterpreter->isBlockingScriptRunning()){
    Engine::instance()->walkTo(mFocussedChar, pos-mScrollOffset, UNSPECIFIED);
  }
  int curCmd = mCursor->getCurrentCommand();
  if (curCmd != mActiveCommand && mLinkObjectInfo.empty()){
    mPrevActiveCommand = mActiveCommand;
    mActiveCommand = curCmd;
    mUseObjectName = "";
    mGiveObjectName = "";
    mLinkObjectInfo = "";
  }
}

void Engine::rightClick(const Vec2i& pos){
  bool leftClickRequired;
  int cmd = mCursor->getNextCommand(leftClickRequired);
  mActiveCommand = cmd;
  mUseObjectName = "";
  mGiveObjectName = "";
  mLinkObjectInfo = "";
  if (leftClickRequired)
    leftClick(pos);
}

void Engine::doubleClick(const Vec2i& pos){
  ExecutionContext* script = NULL;
  Object2D* obj = getObjectAt(pos);
  if (obj != NULL){
    script = obj->getScript();
    if (script != NULL){
      script->setEvent(EVT_DBLCLCK);
    }
  }
}

bool Engine::setFocus(std::string charname, ExecutionContext* reason){
  if (mFocussedChar){
    mRooms.back()->addObject(mFocussedChar);
  }
  if (charname == "none"){
    if (mFocussedChar)
      mLastFocussedChar = mFocussedChar->getName();
    mFocussedChar = NULL;
    return true;
  }
  if (charname == "last"){
    charname = mLastFocussedChar;
  }
  CharacterObject* res = extractCharacter(charname);
  if (res){
    mFocussedChar = res;
    return true;
  }
  /*if (!mLastFocussedChar.empty() && mLastFocussedChar != "none"){
    focusChar(mLastFocussedChar);
    return true;
  }*/
  //load character
  res = loadCharacter(charname, getCharacterClass(charname), true, reason);
  mSaver->removeCharacter(charname);
  if (res){
    mFocussedChar = res;
    return true;
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

void Engine::walkTo(CharacterObject* chr, const Vec2i& pos, LookDir dir){
  Vec2i oldwmpos = chr->getPosition()/mWalkGridSize;
  Vec2i newwmpos = pos/mWalkGridSize;
  std::list<Vec2i> path;
  bool couldReach = aStarSearch(oldwmpos,newwmpos,path); 
  //remove first entry as we are already on that field
  path.pop_front();
  for (std::list<Vec2i>::iterator iter = path.begin(); iter != path.end(); ++iter){
    *iter = *iter*mWalkGridSize+Vec2i(mWalkGridSize/2,mWalkGridSize/2);
  }
  //replace last entry
  if (couldReach){
    if (!path.empty())
      path.pop_back();
    path.push_back(pos);
  }
  chr->setEndLookDir(dir);
  mAnimator->add(chr, path, 3);
}

Object2D* Engine::createItem(const std::string& name){
  Item* it = mData->getItem(name);
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

void Engine::addUIElement(Object2D* elem){
  mUI.push_back(elem);
}

void Engine::setCommand(const std::string& command, bool deleteLinks){
  if (command == "none"){
    int cmd = mData->getProjectSettings()->commands["walkto"];
    mActiveCommand = cmd;
    mCursor->setState(0);
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
      loadRoom(room, false, loadreason);
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
  CharacterObject* character = new CharacterObject(obj->base.state, obj->base.position, realName);
  character->setLightingColor(obj->base.lighting);
  character->setMirrored(obj->mirrored);
  character->setFontID(obj->fontid);
  mFonts->loadFont(obj->fontid);
  character->setTextColor(chbase->textcolor);
  character->setRoom(room);
  //RoomObject* ro = Engine::instance()->getRoom(room);
  //if (ro)
  //  character->setScale(ro->getDepthScale(obj->base.position));
  for (unsigned j = 0; j < chbase->states.size(); ++j){
    int depth = (obj->base.position.y+chbase->states[j].basepoint.y)/mWalkGridSize;
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
  return character;
}

void Engine::keyPress(int key){
  mKeysPressed[key] = true;
  mKeysDown[key] = true;
  switch(key){
    case KEY_F1:
      mSaver->save(SaveStateProvider::saveSlotToPath(1));
      break;
    case KEY_F2:
      mSaver->load(SaveStateProvider::saveSlotToPath(1));
      break;
    case KEY_ESCAPE:
      {
        ExecutionContext* ctx = mInterpreter->getCutscene();
        if (ctx){
          ctx->setSkip();
        }
      }
      break;
  }
}

void Engine::keyRelease(int key){
  mKeysDown[key] = false;
  mKeysPressed[key] = false;
}

void Engine::unloadRooms(){
  setFocus("none", NULL);
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ){
    mRoomsToUnload.push_back(*iter);
    iter = mRooms.erase(iter);
  }
  mMainRoomLoaded = false;
  mSubRoomLoaded = false;
  mTaskbar = NULL;
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
  std::list<BaseBlitObject*> tmp = mBlitQueue;
  mBlitQueue = std::list<BaseBlitObject*>();
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    (*iter)->render();
  }
  GL()vertexPointer(2, GL_SHORT, 0, mVerts);
  GL()texCoordPointer(2, GL_SHORT, 0, mVerts);
  for (std::list<BaseBlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){ 
    (*iter)->blit();
  }
  //mBlitQueue.clear();
  mBlitQueue = tmp;
}
