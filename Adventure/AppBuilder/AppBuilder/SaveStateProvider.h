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
    SaveObject base;
    Color lighting;
    Vec2i scrolloffset;
    std::map<std::string, SaveObject*> objects;
    std::map<std::string, CharSaveObject*> characters;
    //friend std::ostream& operator<<(std::ostream& strm, const SaveRoom& room);
  };

  SaveStateProvider(AdvDocument* data);
  ~SaveStateProvider();
  SaveRoom* getRoom(const std::string name);
  SaveObject* getObject(const std::string& name);
  SaveObject* getOrAddObject(const std::string& name);
  void removeObject(const std::string& name);
  CharSaveObject* getCharacter(const std::string& name);
  CharSaveObject* getOrAddCharacter(const std::string& name);
  CharSaveObject* findCharacter(const std::string& name, std::string& room);
  void removeCharacter(const std::string& name);
  void clear();
  void save(const std::string& name);
  void load(const std::string& name);
  void allowWrites(bool allow) {mAllowWrites = allow;}
  bool isWriteAllowed() {return mAllowWrites;}
  static std::string saveSlotToPath(int slot);
protected:
  AdvDocument* mData;
  std::map<std::string, SaveRoom*> mRooms;
  SaveRoom* mLastRoom;
  bool mAllowWrites;
};

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveRoom& room);
std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveObject& object);
std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::CharSaveObject& chr);
std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveInventory& inv);

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveRoom& room);
std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveObject& object);
std::istream& operator>>(std::istream& strm, SaveStateProvider::CharSaveObject& chr);
std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveInventory& inv);

#endif
