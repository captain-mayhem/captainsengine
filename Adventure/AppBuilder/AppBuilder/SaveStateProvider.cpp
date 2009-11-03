#include "SaveStateProvider.h"

SaveStateProvider::SaveStateProvider(AdvDocument* data) : mData(data) {

}

SaveStateProvider::~SaveStateProvider(){
  for (std::map<std::string,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<std::string,SaveObject*>::iterator objiter = iter->second->objects.begin(); objiter != iter->second->objects.end(); ++objiter){
      delete objiter->second;
    }
    for (std::map<std::string,CharSaveObject*>::iterator chriter = iter->second->characters.begin(); chriter != iter->second->characters.end(); ++chriter){
      delete chriter->second;
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
        CharSaveObject* chr = new CharSaveObject();
        chr->base.state = 0;
        chr->base.position = mData->getRoomCharacters()[i].position;
        save->characters[mData->getRoomCharacters()[i].name] = chr;
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

SaveStateProvider::SaveObject* SaveStateProvider::getOrAddObject(const std::string& name){
  std::map<std::string,SaveObject*>::iterator iter = mLastRoom->objects.find(name);
  if (mLastRoom->objects.empty() || iter == mLastRoom->objects.end()){
    SaveObject* so = new SaveObject;
    mLastRoom->objects.insert(std::make_pair(name,so));
    return so;
  }
  return iter->second;
}

void SaveStateProvider::removeObject(const std::string& name){
  delete mLastRoom->objects[name];
  mLastRoom->objects.erase(name);
}

SaveStateProvider::CharSaveObject* SaveStateProvider::getCharacter(const std::string& name){
  std::map<std::string,CharSaveObject*>::iterator iter = mLastRoom->characters.find(name);
  if (mLastRoom->characters.empty() || iter == mLastRoom->characters.end()){
    return NULL;
  }
  return iter->second;
}

SaveStateProvider::CharSaveObject* SaveStateProvider::getOrAddCharacter(const std::string& name){
  std::map<std::string,CharSaveObject*>::iterator iter = mLastRoom->characters.find(name);
  if (mLastRoom->characters.empty() || iter == mLastRoom->characters.end()){
    CharSaveObject* cso = new CharSaveObject;
    mLastRoom->characters.insert(std::make_pair(name,cso));
    return cso;
  }
  return iter->second;
}

void SaveStateProvider::removeCharacter(const std::string& name){
  delete mLastRoom->characters[name];
  mLastRoom->characters.erase(name);
}
