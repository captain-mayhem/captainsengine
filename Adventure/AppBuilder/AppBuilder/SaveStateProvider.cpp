#include "SaveStateProvider.h"

#include <fstream>
#include <sstream>

#include "Engine.h"
#include "Sound.h"
#include "Particles.h"

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveRoom& room){
  strm << room.base;
  strm << room.scrolloffset.x << " " << room.scrolloffset.y << std::endl;
  strm << room.objects.size() << std::endl;
  for (std::map<std::string,SaveStateProvider::SaveObject*>::const_iterator iter = room.objects.begin(); iter != room.objects.end(); ++iter){
    strm << iter->first << std::endl << *iter->second;
  }
  strm << room.characters.size() << std::endl;
  for (std::map<std::string,SaveStateProvider::CharSaveObject*>::const_iterator iter = room.characters.begin(); iter != room.characters.end(); ++iter){
    strm << iter->first << std::endl << *iter->second;
  }
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveRoom& room){
  strm >> room.base;
  strm >> room.scrolloffset.x >> room.scrolloffset.y;
  int number;
  strm >> number;
  std::string name;
  for (int i = 0; i < number; ++i){
    strm >> name;
    SaveStateProvider::SaveObject* obj = new SaveStateProvider::SaveObject;
    strm >> *obj;
    room.objects[name] = obj;
  }
  strm >> number;
  for (int i = 0; i < number; ++i){
    strm >> name;
    SaveStateProvider::CharSaveObject* chr = new SaveStateProvider::CharSaveObject;
    strm >> *chr;
    room.characters[name] = chr;
  }
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveObject& object){
  strm << object.position.x << " " << object.position.y << " " << object.state << std::endl;
  strm << object.lighting.r << " " << object.lighting.g << " " << object.lighting.b << " " << object.lighting.a << std::endl;
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveObject& object){
  strm >> object.position.x >> object.position.y >> object.state;
  strm >> object.lighting.r >> object.lighting.g >> object.lighting.b >> object.lighting.a;
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::CharSaveObject& chr){
  strm << chr.base;
  strm << chr.mirrored << std::endl;
  strm << chr.inventory;
  strm << chr.fontid << std::endl;
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::CharSaveObject& chr){
  strm >> chr.base >> chr.mirrored;
  strm >> chr.inventory;
  strm >> chr.fontid;
  return strm;
}

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveInventory& inv){
  strm << inv.items.size() << std::endl;
  for (std::map<int, std::vector<std::string> >::const_iterator iter = inv.items.begin(); iter != inv.items.end(); ++iter){
    strm << iter->first << " ";
    strm << iter->second.size();
    for (unsigned i = 0; i < iter->second.size(); ++i){
      strm << " " << iter->second[i];
    }
    strm << std::endl;
  }
  return strm;
}

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveInventory& inv){
  int numInvs, invnum, number;
  std::string itemName;
  strm >> numInvs;
  for (int i = 0; i < numInvs; ++i){
    strm >> invnum >> number;
    for (int j = 0; j < number; ++j){
      strm >> itemName;
      inv.items[invnum].push_back(itemName);
    }
  }
  return strm;
}

SaveStateProvider::SaveStateProvider(AdvDocument* data) : mData(data), mAllowWrites(true) {

}

SaveStateProvider::~SaveStateProvider(){
  clear();
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
    save->base.position = Vec2i();
    save->base.lighting = Color();
    save->scrolloffset = orig->scrolloffset*-1;
    for (unsigned i = 0; i < orig->objects.size(); ++i){
      SaveObject* object = new SaveObject();
      object->lighting = Color();
      object->state = orig->objects[i].state;
      object->position = orig->objects[i].position;
      save->objects[orig->objects[i].name] = object;
    }
    for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
      if (mData->getRoomCharacters()[i].room == name){
        CharacterObject dummy(0, Vec2i(), "");
        dummy.setLookDir(mData->getRoomCharacters()[i].dir);
        CharSaveObject* chr = new CharSaveObject();
        chr->base.lighting = Color();
        chr->base.state = dummy.getState();
        chr->base.position = mData->getRoomCharacters()[i].position;
        chr->mirrored = dummy.isMirrored();
        Character* chbase = mData->getCharacter(mData->getRoomCharacters()[i].character);
        int fontid = chbase->fontid;
        if (fontid == 0)
          fontid = 1;
        chr->fontid = fontid;
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

void SaveStateProvider::clear(){
  for (std::map<std::string,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<std::string,SaveObject*>::iterator objiter = iter->second->objects.begin(); objiter != iter->second->objects.end(); ++objiter){
      delete objiter->second;
    }
    for (std::map<std::string,CharSaveObject*>::iterator chriter = iter->second->characters.begin(); chriter != iter->second->characters.end(); ++chriter){
      delete chriter->second;
    }
    delete iter->second;
  }
  mRooms.clear();
}

void SaveStateProvider::save(const std::string& name){
  std::string focussedcharname;
  if (Engine::instance()->mFocussedChar){
    getRoom(Engine::instance()->mRooms.back()->getName());
    focussedcharname = Engine::instance()->mFocussedChar->getName();
    Engine::instance()->mFocussedChar->save();
  }
  for (std::list<RoomObject*>::iterator iter = Engine::instance()->mRooms.begin(); iter != Engine::instance()->mRooms.end(); ++iter){
    (*iter)->save();
  }
  std::ofstream out(name.c_str());
  //save room data
  out << mRooms.size() << std::endl;
  for (std::map<std::string,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    out << iter->first << std::endl << *iter->second;
  }
  //save loaded rooms
  out << Engine::instance()->mRooms.size();
  for (std::list<RoomObject*>::reverse_iterator iter = Engine::instance()->mRooms.rbegin(); iter != Engine::instance()->mRooms.rend(); ++iter){
    out << " " << (*iter)->getName();
  }
  out << std::endl;
  //focussed char
  CharacterObject* chr = Engine::instance()->getCharacter("self");
  if (chr != NULL)
    out << chr->getName() << std::endl;
  else
    out << "none" << std::endl;
  if (!Engine::instance()->mLastFocussedChar.empty())
    out << Engine::instance()->mLastFocussedChar << std::endl;
  else
    out << "none" << std::endl;
  out << Engine::instance()->mMainRoomLoaded << " " << Engine::instance()->mSubRoomLoaded << std::endl;
  out << Engine::instance()->mShowTaskbar << " " << Engine::instance()->mScreenChange << std::endl;
  out << Engine::instance()->mTextEnabled << " " << Engine::instance()->mFontID << std::endl;
  Engine::instance()->getInterpreter()->save(out);
  SoundEngine::instance()->save(out);
  Engine::instance()->getParticleEngine()->save(out);
  if (Engine::instance()->mFocussedChar)
    removeCharacter(focussedcharname);
  out.close();
}

void SaveStateProvider::load(const std::string& name){
  std::ifstream in(name.c_str());
  if (!in)
    return;
  //clear the current save state;
  clear();
  Engine::instance()->getInterpreter()->executeCutscene(NULL, false);
  Engine::instance()->unloadRooms();
  //load room data
  int numRooms;
  in >> numRooms;
  std::string roomname;
  for (int i = 0; i < numRooms; ++i){
    in >> roomname;
    SaveRoom* room = new SaveRoom;
    in >> *room;
    mRooms[roomname] = room;
  }
  //load rooms to load
  in >> numRooms;
  for (int i = 0; i < numRooms; ++i){
    in >> roomname;
    Engine::instance()->loadRoom(roomname, i != 0, NULL);
  }
  //focussed char
  std::string focussedchar;
  in >> focussedchar;
  in >> Engine::instance()->mLastFocussedChar;
  if (Engine::instance()->mLastFocussedChar == "none"){
    Engine::instance()->mLastFocussedChar = "none";
  }
  Engine::instance()->setFocus(focussedchar, NULL);
  //data
  int tmp;
  in >> Engine::instance()->mMainRoomLoaded >> Engine::instance()->mSubRoomLoaded;
  in >> Engine::instance()->mShowTaskbar >> tmp;
  Engine::instance()->mScreenChange = (ScreenChange)tmp;
  in >> Engine::instance()->mTextEnabled >> Engine::instance()->mFontID;
  Engine::instance()->getInterpreter()->load(in);
  SoundEngine::instance()->load(in);
  Engine::instance()->getParticleEngine()->load(in);
  allowWrites(false);
  in.close();
}

std::string SaveStateProvider::saveSlotToPath(int slot){
  std::ostringstream path;
  path << Engine::instance()->getSettings()->savedir;
  path << "/save" << slot << ".sav";
  return path.str();
}

SaveStateProvider::CharSaveObject* SaveStateProvider::findCharacter(const std::string& name, std::string& room, std::string& realName){
  //check if already present
  for (std::map<std::string,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<std::string,CharSaveObject*>::iterator chriter = iter->second->characters.begin(); chriter != iter->second->characters.end(); ++chriter){
      if (_stricmp(chriter->first.c_str(), name.c_str()) == 0){
        room = iter->first;
        realName = chriter->first;
        return chriter->second;
      }
    }
  }
  for (unsigned i = 0; i < mData->getRoomCharacters().size(); ++i){
    if (_stricmp(mData->getRoomCharacters()[i].name.c_str(), name.c_str()) == 0){
      //load the room with the character into saving
      room = mData->getRoomCharacters()[i].room;
      realName = mData->getRoomCharacters()[i].name;
      SaveRoom* saveroom = getRoom(room);
      std::map<std::string,CharSaveObject*>::iterator iter = saveroom->characters.find(realName);
      if (iter != saveroom->characters.end())
        return iter->second;
      for (std::map<std::string,CharSaveObject*>::iterator iter = saveroom->characters.begin(); iter != saveroom->characters.end(); ++iter){
        if (_stricmp(iter->first.c_str(), name.c_str()) == 0)
          DebugBreak();
          return iter->second;
      }
    }
  }
  return NULL;
}

SaveStateProvider::SaveObject* SaveStateProvider::findObject(const std::string& name, std::string& room){
  //check if already present
  for (std::map<std::string,SaveRoom*>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    for (std::map<std::string,SaveObject*>::iterator objiter = iter->second->objects.begin(); objiter != iter->second->objects.end(); ++objiter){
      if (_stricmp(objiter->first.c_str(), name.c_str()) == 0){
        room = iter->first;
        return objiter->second;
      }
    }
  }
  //load the room with the object into saving
  Object* obj = mData->getObject(name);
  Room* rm = mData->getRoom(obj);
  room = rm->name;
  SaveRoom* saveroom = getRoom(room);
  std::map<std::string,SaveObject*>::iterator iter = saveroom->objects.find(obj->name);
  if (iter != saveroom->objects.end())
    return iter->second;
  DebugBreak();
  return NULL;
}
