#ifndef SAVE_STATE_PROVIDER_H
#define SAVE_STATE_PROVIDER_H

#include "EngineObjects.h"

namespace adv{

class SaveStateProvider{
public:
  struct SaveObject{
    SaveObject() : position(-1000,-1000), state(1), lighting(), name("none") {}
    Vec2i position;
    int state;
    Color lighting;
    std::string name;
  };
  struct SaveInventory{
    std::map<int, std::vector<std::string> > items;
  };
  struct CharSaveObject{
    CharSaveObject() : mirrored(false), fontid(-1) {}
    SaveObject base;
    bool mirrored;
    SaveInventory inventory;
    int fontid;
    float scale;
    bool nozooming;
    std::string walksound;
  };
  struct SaveRoom{
    SaveObject base;
    Vec2i scrolloffset;
    Color overlaylighting;
    std::map<std::string, SaveObject*> objects;
    std::map<std::string, CharSaveObject*> characters;
    std::map<Vec2i, bool> walkmap;
    void clear(){
      for (std::map<std::string, SaveObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter){
        delete iter->second;
      }
      objects.clear();
      for (std::map<std::string, CharSaveObject*>::iterator iter = characters.begin(); iter != characters.end(); ++iter){
        delete iter->second;
      }
      characters.clear();
      walkmap.clear();
    }
    //friend std::ostream& operator<<(std::ostream& strm, const SaveRoom& room);
  };

  SaveStateProvider(AdvDocument* data);
  ~SaveStateProvider();
  SaveRoom* getRoom(const std::string name);
  SaveObject* getObject(const std::string& name);
  SaveObject* getOrAddObject(const std::string& name);
  SaveObject* findObject(const std::string& name, std::string& room);
  void removeObject(const std::string& name);
  CharSaveObject* getCharacter(const std::string& name);
  CharSaveObject* getOrAddCharacter(const std::string& name);
  CharSaveObject* findCharacter(const std::string& name) {std::string dummy; return findCharacter(name, dummy, dummy);}
  CharSaveObject* findCharacter(const std::string& name, std::string& room, std::string& realName);
  void removeCharacter(const std::string& name);
  void clear();
  void save(const std::string& name);
  void load(const std::string& name);
  void allowWrites() {--mNoWrites;}
  bool isWriteAllowed() {return mNoWrites == 0;}
  static std::string saveSlotToPath(int slot);
protected:
  AdvDocument* mData;
  std::map<std::string, SaveRoom*> mRooms;
  SaveRoom* mLastRoom;
  int mNoWrites;
};

std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveRoom& room);
std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveObject& object);
std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::CharSaveObject& chr);
std::ostream& operator<<(std::ostream& strm, const SaveStateProvider::SaveInventory& inv);

std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveRoom& room);
std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveObject& object);
std::istream& operator>>(std::istream& strm, SaveStateProvider::CharSaveObject& chr);
std::istream& operator>>(std::istream& strm, SaveStateProvider::SaveInventory& inv);

}

#endif
