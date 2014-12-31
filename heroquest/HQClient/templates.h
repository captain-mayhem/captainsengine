#ifndef TEMPLATES_HH
#define TEMPLATES_HH

#include <vector>

#include "renderer/model.h"

#include "item.h"
#include "monster.h"
#include "overlay.h"

using std::vector;

class Templates{
  public:
    ~Templates();
    //! init template object
    static void init() { templ = new Templates(); templ->load(); atexit(release); }
    //! release template object
    static void release() { delete templ; templ = NULL; }
    //! load item list
    void load();
    //! get the number of monster types
    inline unsigned numMonsterTypes() {return (unsigned)monsterTypes_.size();}
    //! get monster reference by index
    inline Monster& getMonster(unsigned id) {return monsterTypes_[id-2000];/*This is dangerous*/}
    //! get monster by name
    Monster getMonster(string name);
    //! get the item called "name"
    Item getItem(string name);
    //! get the armory called "name"
    Item getArmory(string name);
    //! get all items of class "type"
    vector<Item> getItems(Item::Type type);
    //! get the spell called "name"
    Item getSpell(string name);
    //! get all spells of class "type"
    vector<Item> getSpells(string type);
    //! returns an item or an armory or a spell called name
    Item searchItem(string name);
    //! get instance
    static inline Templates* instance() {return templ;}
    //! get an overlay object by shortcut
    Overlay* getOverlay(string brev);
    //! get an overlay by id
    inline Overlay& getOverlay(unsigned id) { return *overlays_[id - 4000]; }
    //! get a model
    MeshGeo::Model* getModel(int idx) {return models_[idx];}
  private:
    static Templates* templ;
    //! Constructor
    Templates();
    //! the monsters
    vector<Monster> monsterTypes_; 
    //! the items
    vector<Item> items_;
    //! the armory
    vector<Item> armory_;
    //! all available spells
    vector<Item> spells_;
    //! the overlays
    vector<Overlay*> overlays_;
    //! the template models
    vector<MeshGeo::Model*> models_;
};

#endif
