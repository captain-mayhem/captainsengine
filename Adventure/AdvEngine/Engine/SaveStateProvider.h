#ifndef SAVE_STATE_PROVIDER_H
#define SAVE_STATE_PROVIDER_H

#include <map>
#include <vector>

#include <system/thread.h>

#include "Vector.h"
#include "String.h"
#include "Ids.h"

namespace adv{

class AdvDocument;

class SaveStateProvider{
public:
  struct SaveObject{
    SaveObject() : position(-1000,-1000), state(1), lighting(), name("none") {}
    Vec2i position;
    int state;
    Color lighting;
    String name;
  };
  struct SaveItem{
    String name;
    int count;
  };
  struct SaveInventory{
    SaveInventory() {current = 1;}
    std::map<int, std::vector<SaveItem> > items;
    int current;
  };
  struct CharSaveObject{
    CharSaveObject() : mirrored(false), fontid(-1) {}
    SaveObject base;
    bool mirrored;
    SaveInventory inventory;
    int fontid;
    float scale;
    bool nozooming;
    String walksound;
    String linkedObject;
  };
  struct SaveRoom{
    SaveObject base;
    Vec2i scrolloffset;
    Color overlaylighting;
    std::map<String, SaveObject*> objects;
    std::map<String, CharSaveObject*> characters;
    std::map<Vec2i, bool> walkmap;
    bool doublewalkmap;
    void clear(){
      for (std::map<String, SaveObject*>::iterator iter = objects.begin(); iter != objects.end(); ++iter){
        delete iter->second;
      }
      objects.clear();
      for (std::map<String, CharSaveObject*>::iterator iter = characters.begin(); iter != characters.end(); ++iter){
        delete iter->second;
      }
      characters.clear();
      walkmap.clear();
    }
    float getWalkGridSize();
    //friend std::ostream& operator<<(std::ostream& strm, const SaveRoom& room);
  };

  SaveStateProvider(AdvDocument* data);
  ~SaveStateProvider();
  SaveRoom* getRoom(const std::string name);
  SaveObject* getObject(SaveStateProvider::SaveRoom* room, const std::string& name);
  SaveObject* getOrAddObject(SaveStateProvider::SaveRoom* room, const std::string& name);
  SaveObject* findObject(const std::string& name, std::string& room);
  void removeObject(SaveStateProvider::SaveRoom* room, const std::string& name);
  CharSaveObject* getCharacter(SaveStateProvider::SaveRoom* room, const std::string& name);
  CharSaveObject* getOrAddCharacter(SaveStateProvider::SaveRoom* room, const std::string& name);
  CharSaveObject* findCharacter(const std::string& name) {std::string dummy; std::string dummy2; return findCharacter(name, dummy, dummy2);}
  CharSaveObject* findCharacter(const std::string& name, std::string& room, std::string& realName);
  void removeCharacter(SaveStateProvider::SaveRoom* room, const std::string& name);
  void clear();
  void save(const std::string& name);
  void load(const std::string& name, bool fromScript);
  void allowWrites() {--mNoWrites;}
  bool isWriteAllowed() {return mNoWrites == 0;}
  static std::string saveSlotToPath(int slot);
  void lock() {mMuty.lock();}
  void unlock() {mMuty.unlock();}
protected:
  AdvDocument* mData;
  std::map<String, SaveRoom*> mRooms;
  int mNoWrites;
  CGE::Mutex mMuty;
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
