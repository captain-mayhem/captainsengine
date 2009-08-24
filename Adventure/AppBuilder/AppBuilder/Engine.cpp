#include "Engine.h"

#include <wx/image.h>

#include "AdvDoc.h"

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
  mInterpreter = new PcdkScript();
}

Engine::~Engine(){
  delete mInterpreter;
}

void Engine::initGame(){
  //load cursor
  if (!mData)
    return;
  mFocussedChar = NULL;
  Vec2i res = mData->getProjectSettings()->resolution;
  mWalkGridSize = res.x/32;
  mWalkFields = Vec2i(32,res.y/mWalkGridSize);
  Cursor* cursor = mData->getCursor();
  mCursor = new Object2D(1, Vec2i(0,0));
  for (unsigned j = 0; j < cursor->size(); ++j){
    Animation* anim = new Animation((*cursor)[j].frames, (*cursor)[j].fps, (*cursor)[j].highlight*-1, 20000);
    mCursor->addAnimation(anim);
  }
  Script* startScript = mData->getScript(Script::CUTSCENE,mData->getProjectSettings()->startscript);
  if (startScript){
    PcdkScript::CodeSegment* seg = mInterpreter->parseProgram(startScript->text);
    mInterpreter->execute(seg);
    delete seg;
  }
}

void Engine::exitGame(){
  for (unsigned i = 0; i < mRooms.size(); ++i){
    delete mRooms[i];
  }
  mRooms.clear();
  delete mCursor;
  delete mFocussedChar;
}

std::string Engine::resolveFilename(ResourceID id, std::string resource){
  return mData->getFilename(IMAGE,resource).GetFullPath().c_str();
}

void Engine::render(){
  for (unsigned i = 0; i < mRooms.size(); ++i){
    mRooms[i]->render();
  }
  if (mFocussedChar)
    mFocussedChar->render();
  mCursor->render();
  glVertexPointer(2, GL_SHORT, 0, mVerts);
  glTexCoordPointer(2, GL_SHORT, 0, mVerts);
  for (std::list<BlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){ 
    (*iter)->blit();
  }
  mBlitQueue.clear();
}

bool Engine::loadRoom(std::string name){
  if (!mData)
    return false;
  Room* room = mData->getRoom(name);
  if (!room)
    return false;
  RoomObject* roomobj = new RoomObject();
  roomobj->setBackground(room->background);
  roomobj->setWalkmap(room->walkmap);
  for (unsigned i = 0; i < room->objects.size(); ++i){
    Object2D* object = new Object2D(room->objects[i].state, room->objects[i].position);
    Object* o = mData->getObject(room->objects[i].object);
    //calculate render depth
    int depth;
    if (room->objects[i].layer == 0)
      depth = 0;
    else if (room->objects[i].layer == 1)
      depth = room->objects[i].wm_depth-1;
    else
      depth = 10000;
    for (unsigned j = 0; j < o->states.size(); ++j){
      Animation* anim = new Animation(o->states[j].frames, o->states[j].fps, depth);
      object->addAnimation(anim);
    }
    roomobj->addObject(object);
  }
  //now check for the characters
  for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
    if (mData->getRoomCharacters()[i].room == name){
      Rcharacter ch = mData->getRoomCharacters()[i];
      int state = 0;
      if (ch.dir == BACK)
        state = 1;
      else if (ch.dir == FRONT)
        state = 2;
      else
        state = 3;
      Character* chbase = mData->getCharacter(ch.character);
      CharacterObject* character = new CharacterObject(state, ch.position, ch.name);
      for (unsigned j = 0; j < chbase->states.size(); ++j){
        int depth = (ch.position.y+chbase->states[j].basepoint.y)/mWalkGridSize;
        Animation* anim = new Animation(chbase->states[j].frames, chbase->states[j].fps, depth);
        character->addAnimation(anim);
        character->addBasepoint(chbase->states[j].basepoint);
      }
      roomobj->addObject(character);
    }
  };
  mRooms.push_back(roomobj);
  return true;
}

void Engine::insertToBlit(BlitObject* obj){
  bool placenotfound = true;
  for (std::list<BlitObject*>::iterator iter = mBlitQueue.begin(); iter != mBlitQueue.end(); ++iter){
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

void Engine::leftClick(Vec2i pos){
  if (mFocussedChar){
    Vec2i oldwmpos = mFocussedChar->getPosition()/mWalkGridSize;
    Vec2i newwmpos = pos/mWalkGridSize;
    if (mRooms.back()->isWalkable(newwmpos)){
      mFocussedChar->setPosition(pos);
      mFocussedChar->setDepth(newwmpos.y);
    }
  }
}

bool Engine::setFocus(std::string charname){
  CharacterObject* res = NULL;
  for (unsigned i = 0; i < mRooms.size(); ++i){
    res = mRooms[i]->extractCharacter(charname);
    if (res){
      mFocussedChar = res;
      return true;
    }
  }
  return false;
}
