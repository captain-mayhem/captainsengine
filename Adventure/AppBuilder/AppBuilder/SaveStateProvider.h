#ifndef SAVE_STATE_PROVIDER_H
#define SAVE_STATE_PROVIDER_H

#include "EngineObjects.h"

class SaveStateProvider{
public:
  struct SaveObject{
    SaveObject() : position(-1000,-1000), state(1) {}
    Vec2i position;
    int state;
  };
  struct SaveInventory{
    std::map<int, std::vector<std::string> > items;
  };
  struct CharSaveObject{
    CharSaveObject() : mirrored(false) {}
    SaveObject base;
    bool mirrored;
    SaveInventory inventory;
  };
  struct SaveRoom{
    Color lighting;
    std::map<std::string, SaveObject*> objects;
    std::map<std::string, CharSaveObject*> characters;
  };

  SaveStateProvider(AdvDocument* data);
  ~SaveStateProvider();
  SaveRoom* getRoom(const std::string name);
  SaveObject* getObject(const std::string& name);
  SaveObject* getOrAddObject(const std::string& name);
  void removeObject(const std::string& name);
  CharSaveObject* getCharacter(const std::string& name);
  CharSaveObject* getOrAddCharacter(const std::string& name);
  void removeCharacter(const std::string& name);
  void clear();
protected:
  AdvDocument* mData;
  std::map<std::string, SaveRoom*> mRooms;
  SaveRoom* mLastRoom;
};

#endif
