#include "Engine.h"

#include <wx/image.h>

#include "AdvDoc.h"

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
}

Engine::~Engine(){
}

void Engine::initGame(){
  //load cursor
  if (!mData)
    return;
  Cursor* cursor = mData->getCursor();
  mCursor = new Object2D(1, Vec2i(0,0));
  mCursor->blits.resize(cursor->size());
  for (unsigned j = 0; j < cursor->size(); ++j){
    for (unsigned k = 0; k < (*cursor)[j].frames.size(); ++k){
      BlitGroup* bg = new BlitGroup((*cursor)[j].frames[k], (*cursor)[j].highlight*-1, 20000);
      mCursor->blits[j].push_back(bg);
    }
  }
  //insertToBlit(mCursor->blits[mCursor->state-1][0]);
}

void Engine::exitGame(){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    delete mObjects[i];
  }
  mObjects.clear();
  delete mCursor;
}

std::string Engine::resolveFilename(ResourceID id, std::string resource){
  return mData->getFilename(IMAGE,resource).GetFullPath().c_str();
}

void Engine::render(){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    mObjects[i]->render();
  }
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
  Object2D* roomobj = new Object2D(1, Vec2i(0,0));
  BlitGroup* grp = new BlitGroup(room->background, Vec2i(0,0), -1);
  roomobj->blits.resize(1);
  roomobj->blits[0].push_back(grp);
  mObjects.push_back(roomobj);
  for (unsigned i = 0; i < room->objects.size(); ++i){
    Object2D* object = new Object2D(room->objects[i].state, room->objects[i].position);
    Object* o = mData->getObject(room->objects[i].object);
    //calculate render depth
    int depth;
    if (room->objects[i].layer == 1)
      depth = 0;
    else if (room->objects[i].layer == 2)
      depth = room->objects[i].wm_depth;
    else
      depth = 10000;
    object->blits.resize(o->states.size());
    for (unsigned j = 0; j < object->blits.size(); ++j){
      for (unsigned k = 0; k < o->states[j].frames.size(); ++k){
        BlitGroup* group = new BlitGroup(o->states[j].frames[k].names, o->states[j].frames[k].offsets, depth);
        object->blits[j].push_back(group);
      }
    }
    mObjects.push_back(object);
  }
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
