#include "Loader.h"

#include "Engine.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "ScriptFunc.h"

TR_CHANNEL(ADV_ResLoader);

using namespace adv;

class RoomLoadedEvent : public Event{
public:
  RoomLoadedEvent(RoomObject* room, bool isSubRoom, ExecutionContext* loadreason, ScreenChange screenchange, int fading) :
      mRoom(room), mIsSubRoom(isSubRoom), mLoadreason(loadreason), mScreenchange(screenchange), mFading(fading) {}
      virtual Event* execute(){
        delete mRoom;
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
#if 1
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
      SaveStateProvider::SaveObject* saveobj = Engine::instance()->getSaver()->getObject(room->objects[i].name);
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
    for (std::map<std::string, SaveStateProvider::CharSaveObject*>::iterator iter = save->characters.begin(); iter != save->characters.end(); ++iter){
      Rcharacter ch;
      for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
        if (mData->getRoomCharacters()[i].name == iter->first){
          ch = mData->getRoomCharacters()[i];
          break;
        }
      }
      CharacterObject* character = NULL;//TODO loadCharacter(ch.name, ch.character, false, mLoadreason);
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
#endif
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
  mMutex.lock();
  while(!mQReq.empty()){
    Event* evt = mQReq.getEvent();
    mMutex.unlock();
    Event* res = evt->execute();
    delete evt;
    mMutex.lock();
    if (res != NULL)
      mQRes.addEvent(res);
  }
  mCond.wait(mMutex);
  mMutex.unlock();
  return true;
}

bool ResLoader::handleResultEvent(){
  mMutex.lock();
  if (!mQRes.empty()){
    Event* evt = mQRes.getEvent();
    mMutex.unlock();
    evt->execute();
    delete evt;
    mMutex.lock();
  }
  bool empty = mQRes.empty();
  mMutex.unlock();
  return !empty;
}

void ResLoader::loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading, int depthoffset){
  LoadRoomEvent* lre = new LoadRoomEvent(name, isSubRoom, loadreason, change, fading, depthoffset);
  lre->setData(mData, mCompiler);
  mMutex.lock();
  mQReq.addEvent(lre);
  mCond.signal();
  mMutex.unlock();
}