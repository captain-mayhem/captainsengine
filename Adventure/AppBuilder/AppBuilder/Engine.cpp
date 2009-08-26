#include "Engine.h"

#include <functional>
#include <numeric>

#include <wx/image.h>

#include "AdvDoc.h"

Engine* Engine::mInstance = NULL;

Engine::Engine() : mData(NULL){
  mVerts[0] = 0; mVerts[1] = 1;
  mVerts[2] = 0; mVerts[3] = 0;
  mVerts[4] = 1; mVerts[5] = 1;
  mVerts[6] = 1; mVerts[7] = 0;
  mInterpreter = new PcdkScript();
  mAnimator = new Animator();
}

Engine::~Engine(){
  delete mInterpreter;
  delete mAnimator;
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
  mAnimator->clear();
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
  unsigned interval = mTimer.Time();
  mTimer.Start(0);
  if (mTimeIntervals.size() > 50)
    mTimeIntervals.pop_back();
  mTimeIntervals.push_front(interval);
  interval = std::accumulate(mTimeIntervals.begin(), mTimeIntervals.end(), 0)/(unsigned)mTimeIntervals.size();
  //some animation stuff
  mAnimator->update(interval);

  //build blit queue
  for (unsigned i = 0; i < mRooms.size(); ++i){
    mRooms[i]->render();
  }
  if (mFocussedChar)
    mFocussedChar->render();
  mCursor->render();

  //render the stuff
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
    mAnimator->add(mFocussedChar, path, 3);
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
          if (y.pos.y >= mWalkFields.y)
            continue;
          break;
        case 2:
          y.pos.x -= 1;
          if (y.pos.x < 0)
            continue;
          break;
        case 3:
          y.pos.x += 1;
          if (y.pos.x >= mWalkFields.x)
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
