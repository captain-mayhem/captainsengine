#include "SaveStateProvider.h"

SaveStateProvider::SaveStateProvider(AdvDocument* data) : mData(data) {

}

SaveStateProvider::~SaveStateProvider(){
  for (std::map<std::string,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<std::string,SaveObject*>::iterator objiter = iter->second->objects.begin(); objiter != iter->second->objects.end(); ++objiter){
      delete objiter->second;
    }
    delete iter->second;
  }
}

SaveStateProvider::SaveRoom* SaveStateProvider::getRoom(const std::string name){
  mLastRoom = NULL;
  std::map<std::string,SaveRoom*>::iterator iter = mRooms.find(name);
  if (mRooms.empty() || iter == mRooms.end()){
    //let's see if we find the room in the original
    Room* orig = mData->getRoom(name);
    if (!orig)
      return NULL;
    SaveRoom* save = new SaveRoom();
    save->lighting = Color();
    for (unsigned i = 0; i < orig->objects.size(); ++i){
      SaveObject* object = new SaveObject();
      object->state = orig->objects[i].state;
      object->position = orig->objects[i].position;
      save->objects[orig->objects[i].name] = object;
    }
    for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
      if (mData->getRoomCharacters()[i].room == name){
        SaveObject* chr = new SaveObject();
        chr->state = 0;
        chr->position = mData->getRoomCharacters()[i].position;
        save->objects[mData->getRoomCharacters()[i].name] = chr;
      }
    }
    mRooms[name] = save;
    mLastRoom = save;
    return save;
  }
  mLastRoom = iter->second;
  return iter->second;
}

SaveStateProvider::SaveObject* SaveStateProvider::getObject(const std::string& name){
  std::map<std::string,SaveObject*>::iterator iter = mLastRoom->objects.find(name);
  if (mLastRoom->objects.empty() || iter == mLastRoom->objects.end()){
    return NULL;
  }
  return iter->second;
}

void SaveStateProvider::removeObject(const std::string& name){
  delete mLastRoom->objects[name];
  mLastRoom->objects.erase(name);
}
