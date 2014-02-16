#include "Loader.h"

#include "Engine.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "ScriptFunc.h"
#include <system/allocation.h>

TR_CHANNEL(ADV_ResLoader);

using namespace adv;

class RoomLoadedEvent : public Event{
public:
  RoomLoadedEvent(RoomObject* room, bool isSubRoom, ExecutionContext* loadreason, ScreenChange screenchange, int fading) :
      mRoom(room), mIsSubRoom(isSubRoom), mLoadreason(loadreason), mScreenchange(screenchange), mFading(fading) {}
      virtual Event* execute(){
        Engine::instance()->insertRoom(mRoom, mIsSubRoom, mLoadreason, mScreenchange, mFading);
        //delete mRoom;
        return NULL;
      }
private:
  RoomObject* mRoom;
  bool mIsSubRoom;
  ExecutionContext* mLoadreason;
  ScreenChange mScreenchange;
  int mFading;
};

class LoadRoomEvent : public Event{
public:
  LoadRoomEvent(const std::string& roomname, bool subroom, ExecutionContext* reason, ScreenChange change, int fade, int depthoffset) :
      mName(roomname), mIsSubRoom(subroom), mLoadreason(reason), mScreenchange(change), mFading(fade), mDepthOffset(depthoffset) {}
      void setData(AdvDocument* data, PcdkScript* cc) {mData = data; mCompiler = cc;}
  virtual Event* execute(){
    TR_USE(ADV_ResLoader);
    TR_INFO("loading room %s with screenchange %i", mName.c_str(), mScreenchange);
    Room* room = mData->getRoom(mName);
    SaveStateProvider::SaveRoom* save = Engine::instance()->getSaver()->getRoom(room->name);
    if (!room || !save)
      return NULL;
    /*int depthoffset = 0;
    if (mIsSubRoom){
      depthoffset = (int)mRooms.size()*1000;
      if (!mMainRoomLoaded)
        depthoffset += 1000;
    }*/
    Room* rm = mData->getRoom(mName);
    RoomObject* roomobj = new RoomObject(save->base.state, /*save->base.position*/Vec2i(0,0), room->size, room->name, rm->depthmap, rm->doublewalkmap);
    roomobj->setParallaxBackground(room->parallaxbackground, mDepthOffset+DEPTH_PARALLAX_BACKGROUND);
    //anywhere room is not allowed to have background
    if (_stricmp(room->name.c_str(), mData->getProjectSettings()->anywhere_room.c_str()) != 0)
      roomobj->setBackground(room->background, mDepthOffset+DEPTH_BACKGROUND);
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
      ExecutionContext* scr = mCompiler->parseProgram(wmscripts[i].second->text);
      if (scr != NULL)
        roomobj->addWalkmapScript(wmscripts[i].first-Vec2i(1,1), scr);
    }
    for (unsigned i = 0; i < room->objects.size(); ++i){
      Object* o = mData->getObject(room->objects[i].object);
      SaveStateProvider::SaveObject* saveobj = Engine::instance()->getSaver()->getObject(save, room->objects[i].name);
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
      object->setDepth(depth+mDepthOffset);
      //object->setScale(roomobj->getDepthScale(saveobj->position));
      for (unsigned j = 0; j < o->states.size(); ++j){
        Animation* anim = new Animation(o->states[j].frames, o->states[j].fps, depth+mDepthOffset);
        object->addAnimation(anim);
      }
      object->setLighten(o->lighten);
      //check for object scripts
      Script* script = mData->getScript(Script::OBJECT,room->objects[i].name+";"+room->name);
      if (script){
        ExecutionContext* scr = mCompiler->parseProgram(script->text);
        if (scr){
          object->setScript(scr);
        }
      }
      roomobj->addObject(object);
    }
    //now check for the characters
    for (std::map<String, SaveStateProvider::CharSaveObject*>::iterator iter = save->characters.begin(); iter != save->characters.end(); ++iter){
      Rcharacter ch;
      for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
        if (iter->first == mData->getRoomCharacters()[i].name){
          ch = mData->getRoomCharacters()[i];
          break;
        }
      }
      CharacterObject* character = Engine::instance()->loadCharacter(ch.name, ch.character, mLoadreason);
      if (character){
        character->setScale(roomobj->getDepthScale(character->getPosition()));
        roomobj->addObject(character);
      }
    };
    //load mirror objects
    for (unsigned i = 0; i < rm->fxshapes.size(); ++i){
      if (rm->fxshapes[i].active){
        if (rm->fxshapes[i].effect == FXShape::WALL_MIRROR || rm->fxshapes[i].effect == FXShape::FLOOR_MIRROR){
          int depth = rm->fxshapes[i].effect == FXShape::FLOOR_MIRROR ? DEPTH_MIRROR_BACK : rm->fxshapes[i].depth-1+mDepthOffset;
          //TODO realize
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
      ExecutionContext* scr = mCompiler->parseProgram(script->text);
      if (scr != NULL){
        roomobj->setScript(scr);
        scr->setEvent(EVT_ENTER);
        //mInterpreter->executeImmediately(scr);
      }
    }
    //inventory display
    if (room->invsize != Vec2i()){
      InventoryDisplay* disp = new InventoryDisplay(room->invpos, room->invsize, room->invscale, DEPTH_ITEM+mDepthOffset);
      roomobj->setInventory(disp);
    }
    return new RoomLoadedEvent(roomobj, mIsSubRoom, mLoadreason, mScreenchange, mFading);
  }
private:
  std::string mName;
  bool mIsSubRoom;
  ExecutionContext* mLoadreason;
  ScreenChange mScreenchange;
  int mFading;
  int mDepthOffset;
  AdvDocument* mData;
  PcdkScript* mCompiler;
};

class InsertCharacterEvent : public Event{
public:
  InsertCharacterEvent(CharacterObject* chr, ExecutionContext* reason, const std::string& room, const Vec2i& pos, LookDir dir) :
      mCharacter(chr), mReason(reason), mRoom(room), mPos(pos), mDir(dir) {}
  virtual Event* execute(){
    if (mReason)
      mReason->resume();
    if (mCharacter){
      SaveStateProvider::SaveRoom* sr = Engine::instance()->getSaver()->getRoom(mCharacter->getRoom());
      Engine::instance()->getSaver()->removeCharacter(sr, mCharacter->getName());
      mCharacter->realize();
    }
    Engine::instance()->insertCharacter(mCharacter, mRoom, mPos, mDir);
    return NULL;
  }
private:
  CharacterObject* mCharacter;
  ExecutionContext* mReason;
  std::string mRoom;
  Vec2i mPos;
  LookDir mDir;
};

class BeamCharacterEvent : public Event{
public:
  BeamCharacterEvent(const std::string& name, ExecutionContext* reason, const std::string& room, const Vec2i& pos, LookDir dir) :
      mName(name), mReason(reason), mRoom(room), mPos(pos), mDir(dir) {}
  virtual Event* execute(){
    CharacterObject* obj = Engine::instance()->loadCharacter(mName, Engine::instance()->getCharacterClass(mName), mReason);
    InsertCharacterEvent* ice = new InsertCharacterEvent(obj, mReason, mRoom, mPos, mDir);
    return ice;
  }
private:
  std::string mName;
  ExecutionContext* mReason;
  std::string mRoom;
  Vec2i mPos;
  LookDir mDir;
};

class ChangeFocusEvent : public Event{
public:
  ChangeFocusEvent(const std::string& chr, ExecutionContext* ctx, Event* loadevt) : mCharacter(chr), mReason(ctx), mRoomLoadEvt(loadevt) {}
  virtual Event* execute(){
    mRoomLoadEvt->execute();
    delete mRoomLoadEvt;
    Engine::instance()->changeFocus(mCharacter, mReason);
    return NULL;
  }
private:
  std::string mCharacter;
  ExecutionContext* mReason;
  Event* mRoomLoadEvt;
};

class SetFocusEvent : public Event{
public:
  SetFocusEvent(const std::string& name, ExecutionContext* reason) : mName(name), mReason(reason) {}
  void setData(AdvDocument* data, PcdkScript* cc) {mData = data; mCompiler = cc;}
  virtual Event* execute(){
    std::string room;
    std::string realName;
    SaveStateProvider::CharSaveObject* obj = Engine::instance()->getSaver()->findCharacter(mName, room, realName);
    if (!obj){
      TR_USE(ADV_ResLoader);
      TR_BREAK("Character %s not found", mName.c_str());
    }
    LoadRoomEvent lre(room, false, mReason, Engine::instance()->getScreenChange(), 0, 0);
    lre.setData(mData, mCompiler);
    Event* loadret = lre.execute();
    return new ChangeFocusEvent(realName, mReason, loadret);
  }
private:
  std::string mName;
  ExecutionContext* mReason;
  AdvDocument* mData;
  PcdkScript* mCompiler;
};

ResLoader::ResLoader() : mData(NULL), mCompiler(NULL) {

}

ResLoader::~ResLoader(){
  //delete mCompiler;
}

void ResLoader::setData(AdvDocument* data){
  mData = data;
  //mCompiler = new PcdkScript(data); //TODO instance access in parseLangArg
  //ScriptFunctions::registerFunctions(mCompiler);
  mCompiler = Engine::instance()->getInterpreter();
}

bool ResLoader::run(){
  TR_USE(ADV_ResLoader);
  mMutex.lock();
  while(!mQReq.empty()){
    Event* evt = mQReq.getEvent();
    mMutex.unlock();
    Event* res = evt->execute();
    delete evt;
    if (res != NULL){
      mResMutex.lock();
      mQRes.addEvent(res);
      mResCond.signal();
      mResMutex.unlock();
    }
    else{
      TR_BREAK("Event produced no result");
    }
    mMutex.lock();
    mQReq.popEvent();
  }
  mCond.wait(mMutex);
  mMutex.unlock();
  return true;
}

bool ResLoader::handleResultEvent(){
  mResMutex.lock();
  if (!mQRes.empty()){
    Event* evt = mQRes.getEvent();
    mResMutex.unlock();
    evt->execute();
    delete evt;
    mResMutex.lock();
    mQRes.popEvent();
  }
  bool empty = mQRes.empty();
  mResMutex.unlock();
  return !empty;
}

void ResLoader::waitUntilFinished(){
  TR_USE(ADV_ResLoader);
  while(1){
    mMutex.lock();
    unsigned reqCount = mQReq.size();
    mResMutex.lock();
    mMutex.unlock();
    if (reqCount+mQRes.size() == 0){
      mResMutex.unlock();
      return;
    }
    if (mQRes.size() == 0){
      if (mResCond.waitTimeout(mResMutex, 1000))
        TR_WARN("timeout occurred!");
    }
    mResMutex.unlock();
    handleResultEvent();
  }
}

void ResLoader::loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading, int depthoffset){
  LoadRoomEvent* lre = new LoadRoomEvent(name, isSubRoom, loadreason, change, fading, depthoffset);
  lre->setData(mData, mCompiler);
  mMutex.lock();
  mQReq.addEvent(lre);
  mCond.signal();
  mMutex.unlock();
}

void ResLoader::beamCharacter(const std::string& name, ExecutionContext* reason, const std::string& room, const Vec2i& pos, LookDir dir){
#if 1
  if (reason)
    reason->suspend(0, NULL);
  BeamCharacterEvent* bce = new BeamCharacterEvent(name, reason, room, pos, dir);
  mMutex.lock();
  mQReq.addEvent(bce);
  mCond.signal();
  mMutex.unlock();
#endif
#if 0
  BeamCharacterEvent bce(name, reason, room, pos, dir);
  Event* ret = bce.execute();
  ret->execute();
  delete ret;
#endif
}

void ResLoader::setFocus(const std::string& name, ExecutionContext* reason){
  if (reason)
    reason->suspend(0, NULL);
  SetFocusEvent* sfe = new SetFocusEvent(name, reason);
  sfe->setData(mData, mCompiler);
  mMutex.lock();
  mQReq.addEvent(sfe);
  mCond.signal();
  mMutex.unlock();
}
