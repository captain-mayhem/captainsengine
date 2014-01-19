#include "Loader.h"

#include "Engine.h"
#include "SaveStateProvider.h"

TR_CHANNEL(ADV_ResLoader);

using namespace adv;

class LoadRoomEvent : public Event{
public:
  LoadRoomEvent(const std::string& roomname, bool subroom, ExecutionContext* reason, ScreenChange change, int fade) :
      mName(roomname), mIsSubRoom(subroom), mLoadreason(reason), mScreenchange(change), mFading(fade) {}
  virtual void execute(){
#if 0
    TR_USE(ADV_ResLoader);
    TR_INFO("loading room %s with screenchange %i", mName.c_str(), mScreenchange);
    Room* room = mData->getRoom(mName);
    SaveStateProvider::SaveRoom* save = Engine::instance()->getSaver()->getRoom(room->name);
    if (!room || !save)
      return;
    int depthoffset = 0;
    if (mIsSubRoom){
      depthoffset = (int)mRooms.size()*1000;
      if (!mMainRoomLoaded)
        depthoffset += 1000;
    }
    Room* rm = mData->getRoom(mName);
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
      CharacterObject* character = loadCharacter(ch.name, ch.character, false, mLoadreason);
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
#endif
  }
private:
  std::string mName;
  bool mIsSubRoom;
  ExecutionContext* mLoadreason;
  ScreenChange mScreenchange;
  int mFading;
  AdvDocument* mData;
};

bool ResLoader::run(){
  mMutex.lock();
  while(!mQ.empty()){
    Event* evt = mQ.getEvent();
    mMutex.unlock();
    evt->execute();
    mMutex.lock();
  }
  mCond.wait(mMutex);
  mMutex.unlock();
  return true;
}

void ResLoader::loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading){
  LoadRoomEvent* lre = new LoadRoomEvent(name, isSubRoom, loadreason, change, fading);
  mMutex.lock();
  mQ.addEvent(lre);
  mCond.signal();
  mMutex.unlock();
}
