#include "Engine.h"

#include <functional>
#include <numeric>
#include <queue>

#include <wx/image.h>

#include "AdvDoc.h"
#include "SaveStateProvider.h"

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL), mInitialized(false){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
  mAnimator = new Animator();
  mFonts = NULL;
  mInterpreter = NULL;
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

void Engine::initGame(){
  if (!mData)
    return;
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
    Animation* anim = new Animation((*cursor)[j].frames, (*cursor)[j].fps, (*cursor)[j].highlight*-1, 20000);
    mCursor->addAnimation(anim, (*cursor)[j].command-1);
  }
  //load fonts
  mFonts->loadFont(0);
  mFonts->loadFont(1);
  mActiveCommand = 0;
  //load anywhere room
  if (mData->getProjectSettings()->anywhere_room != ""){
    loadRoom(mData->getProjectSettings()->anywhere_room, true);
  }
  //load and execute start script
  mInitScript = NULL;
  Script* startScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->startscript);
  if (startScript){
    mInitScript = mInterpreter->parseProgram(startScript->text);
    mInterpreter->execute(mInitScript, true);
  }
  mInitialized = true;
}

void Engine::exitGame(){
  if (!mInitialized)
    return;
  mInitialized = false;
  delete mInitScript;
  mAnimator->clear();
  for (std::list<RoomObject*>::iterator iter = mRoomsToUnload.begin(); iter != mRoomsToUnload.end(); ++iter){
    delete *iter;
  }
  mRoomsToUnload.clear();
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    delete *iter;
  }
  mRooms.clear();
  mMainRoomLoaded = false;
  delete mCursor;
  delete mFocussedChar;
  mFonts->unloadFont(1);
  mFonts->unloadFont(0);
}

wxImage Engine::getImage(const std::string& name){
  return mData->getImage(name);
}

//TODO move into utility
unsigned roundToPowerOf2(unsigned x){
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

GLuint Engine::genTexture(const wxImage& image, Vec2i& size, Vec2f& scale, const wxImage* alphaimage){
  GLuint tex;
  size.x = image.GetWidth();
  size.y = image.GetHeight();
  Vec2i pow2(roundToPowerOf2(size.x), roundToPowerOf2(size.y));
  scale.x = ((float)size.x)/pow2.x;
  scale.y = ((float)size.y)/pow2.y;
  unsigned totalsize = size.x*size.y;
  unsigned char* rgb = image.GetData();
  unsigned char* alpha = image.GetAlpha();
  GLubyte* buffer = new GLubyte[totalsize*4];
  for (unsigned i = 0; i < totalsize; ++i){
    buffer[4*i] = rgb[3*i];
    buffer[4*i+1] = rgb[3*i+1];
    buffer[4*i+2] = rgb[3*i+2];
    if (alpha)
      buffer[4*i+3] = alpha[i];
    else if (alphaimage){
      buffer[4*i+3] = alphaimage->GetRed(i%size.x, i/size.x);
    }
    else{
      if (rgb[3*i] == 0xFF && rgb[3*i+1] == 0x0 && rgb[3*i+2] == 0xFF){
        buffer[4*i+3] = 0;
      }
      else{
        buffer[4*i+3] = 255;
      }
    }
  }
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  delete buffer;
  return tex;
}

void Engine::render(){
  if (!mInitialized)
    return;
  //timing
  unsigned interval = mTimer.Time();
  mTimer.Start(0);
  if (mTimeIntervals.size() > 10)
    mTimeIntervals.pop_back();
  mTimeIntervals.push_front(interval);
  interval = std::accumulate(mTimeIntervals.begin(), mTimeIntervals.end(), 0)/(unsigned)mTimeIntervals.size();
  
  //unload rooms
  while (!mRoomsToUnload.empty()){
    mRoomsToUnload.front()->save();
    delete mRoomsToUnload.front();
    mRoomsToUnload.pop_front();
  }

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
      script->setEvent(EVT_LOOP2);
    }
  }
  if (mRooms.size() > 0 && mFocussedChar){ //walkmap
    Vec2i pos = mFocussedChar->getPosition()/mWalkGridSize;
    mRooms.back()->walkTo(pos);
  }

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
    (*iter)->render();
  }
  if (mFocussedChar)
    mFocussedChar->render();
  mCursor->render();

  //command handling
  Vec2i res = mData->getProjectSettings()->resolution;
  std::string text = mData->getProjectSettings()->pretty_commands[mActiveCommand];
  if (!mObjectInfo.empty()){
    text += " "+mObjectInfo;
    mObjectInfo.clear();
  }
  Vec2i offset = mFonts->getTextExtent(text, 1);
  mFonts->render(res.x/2-offset.x/2, res.y-offset.y, text, 1);

  mFonts->prepareBlit(interval);

  //render the stuff
  glVertexPointer(2, GL_SHORT, 0, mVerts);
  glTexCoordPointer(2, GL_SHORT, 0, mVerts);
  for (std::list<BaseBlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){ 
    (*iter)->blit();
  }
  mBlitQueue.clear();
}

bool Engine::loadRoom(std::string name, bool isSubRoom){
  if (!mData)
    return false;
  Room* room = mData->getRoom(name);
  SaveStateProvider::SaveRoom* save = mSaver->getRoom(name);
  if (!room || !save)
    return false;
  if (mMainRoomLoaded && !isSubRoom){
    unloadRoom(mRooms.back());
  }
  int depthoffset = 0;
  if (isSubRoom){
    depthoffset = (int)mRooms.size()*1000;
    if (!mMainRoomLoaded)
      depthoffset += 1000;
  }
  RoomObject* roomobj = new RoomObject(room->size, name);
  roomobj->setBackground(room->background, depthoffset-1);
  roomobj->setLightingColor(save->lighting);
  roomobj->setWalkmap(room->walkmap);
  //check for walkmap scripts
  std::vector<std::pair<Vec2i,Script*> > wmscripts = mData->getWMScripts(name);
  for (unsigned i = 0; i < wmscripts.size(); ++i){
    ExecutionContext* scr = mInterpreter->parseProgram(wmscripts[i].second->text);
    roomobj->addWalkmapScript(wmscripts[i].first-Vec2i(1,1), scr);
  }
  for (unsigned i = 0; i < room->objects.size(); ++i){
    Object* o = mData->getObject(room->objects[i].object);
    SaveStateProvider::SaveObject* saveobj = mSaver->getObject(room->objects[i].name);
    if (!saveobj)
      continue;
    Object2D* object = new Object2D(saveobj->state, saveobj->position, o->size, o->name);
    //calculate render depth
    int depth;
    if (room->objects[i].layer == 0)
      depth = 0;
    else if (room->objects[i].layer == 1)
      depth = room->objects[i].wm_depth-1;
    else
      depth = 999;
    for (unsigned j = 0; j < o->states.size(); ++j){
      Animation* anim = new Animation(o->states[j].frames, o->states[j].fps, depth+depthoffset);
      object->addAnimation(anim);
    }
    //check for object scripts
    Script* script = mData->getScript(Script::OBJECT,room->objects[i].object+";"+name);
    if (script){
      ExecutionContext* scr = mInterpreter->parseProgram(script->text);
      object->setScript(scr);
      mInterpreter->execute(scr, false);
    }
    roomobj->addObject(object);
  }
  //now check for the characters
  for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
    if (mData->getRoomCharacters()[i].room == name){
      Rcharacter ch = mData->getRoomCharacters()[i];
      Character* chbase = mData->getCharacter(ch.character);
      SaveStateProvider::SaveObject* saveobj = mSaver->getObject(ch.name);
      if (!saveobj)
        continue;
      CharacterObject* character = new CharacterObject(0, ch.position, ch.name);
      character->setLookDir(ch.dir);
      character->setFontID(chbase->fontid+1);
      character->setTextColor(chbase->textcolor);
      for (unsigned j = 0; j < chbase->states.size(); ++j){
        int depth = (ch.position.y+chbase->states[j].basepoint.y)/mWalkGridSize;
        Animation* anim = new Animation(chbase->states[j].frames, chbase->states[j].fps, depth);
        character->addAnimation(anim);
        character->addBasepoint(chbase->states[j].basepoint, chbase->states[j].size);
      }
      Script* script = mData->getScript(Script::CHARACTER,ch.name);
      if (script){
        ExecutionContext* scr = mInterpreter->parseProgram(script->text);
        character->setScript(scr);
        mInterpreter->execute(scr, false);
      }
      roomobj->addObject(character);
    }
  };
  //load room script
  Script* script = mData->getScript(Script::ROOM,name);
  if (script){
    ExecutionContext* scr = mInterpreter->parseProgram(script->text);
    if (scr != NULL){
      roomobj->setScript(scr);
      mInterpreter->execute(scr, false);
      scr->setEvent(EVT_ENTER);
    }
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
  return true;
}

void Engine::unloadRoom(RoomObject* room){
  if (room == NULL)
    room = mRooms.front();
  ExecutionContext* ctx = room->getScript();
  if (ctx){
    ctx->setEvent(EVT_EXIT);
    mInterpreter->executeImmediately(ctx);
  }
  mRoomsToUnload.push_back(room);
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
      script->setEvent((EngineEvent)mActiveCommand);
      script->setStepEndHandler(PcdkScript::clickEndHandler, new Vec2i(pos-mScrollOffset));
    }
    else if (mFocussedChar && !mSubRoomLoaded){
      Engine::instance()->walkTo(mFocussedChar, pos-mScrollOffset, UNSPECIFIED);
    }
  }
  else if (mFocussedChar && !mSubRoomLoaded){
    Engine::instance()->walkTo(mFocussedChar, pos-mScrollOffset, UNSPECIFIED);
  }
}

void Engine::rightClick(const Vec2i& pos){
  int cmd = mCursor->getNextCommand();
  mActiveCommand = cmd;
}

bool Engine::setFocus(std::string charname){
  CharacterObject* res = NULL;
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    res = (*iter)->extractCharacter(charname);
    if (res){
      mFocussedChar = res;
      return true;
    }
  }
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
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    Object2D* ret = (*iter)->getObjectAt(pos);
    if (ret != NULL)
      return ret;
    if (mSubRoomLoaded)
      break;
  }
  return NULL;
}

Object2D* Engine::getObject(const std::string& name){
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    Object2D* ret = (*iter)->getObject(name);
    if (ret != NULL)
      return ret;
  }
  return NULL;
}

CharacterObject* Engine::getCharacter(const std::string& name){
  if (name == "self")
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

RoomObject* Engine::getRoom(const std::string& name){
  for (std::list<RoomObject*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if ((*iter)->getName() == name){
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
