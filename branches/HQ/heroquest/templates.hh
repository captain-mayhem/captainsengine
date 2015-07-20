#ifndef TEMPLATES_HH
#define TEMPLATES_HH

#include <vector>
#include "item.hh"
#include "monster.hh"
#include "overlay.hh"

using std::vector;

class Templates{
  public:
    ~Templates();
    //! init template object
    static void init() { templ = new Templates(); templ->load(); }
    //! load item list
    void load();
    //! get the number of monster types
    inline unsigned numMonsterTypes() {return monsterTypes_.size();}
    //! get monster reference by index
    inline Monster& getMonster(int idx) {return monsterTypes_[idx];}
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
};

#endif
