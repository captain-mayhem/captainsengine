#ifndef SAVE_STATE_PROVIDER_H
#define SAVE_STATE_PROVIDER_H

#include "EngineObjects.h"

class SaveStateProvider{
public:
  struct SaveObject{
    Vec2i position;
    int state;
  };
  struct CharSaveObject{
    SaveObject base;
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
protected:
  AdvDocument* mData;
  std::map<std::string, SaveRoom*> mRooms;
  SaveRoom* mLastRoom;
};

#endif
