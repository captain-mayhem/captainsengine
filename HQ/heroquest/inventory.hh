#ifndef INVENTORY_HH
#define INVENTORY_HH

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include "array.hh"
#include "item.hh"

using std::map;
using std::vector;
using std::ifstream;
using std::ofstream;

class Inventory{
  public:
    //! Constructor
    Inventory();
    //! Copy Constructor
    Inventory(const Inventory& i);
    //! Destuctor
    ~Inventory() {}
    //! add item to inventory
    void addItem(Item it);
    //! delete item from inventory
    bool deleteItem(string name);
    //! use item from inventory
    bool useItem(string name, Vector2D pos, Vector2D target);
    //! mounts an armory on the body part
    bool wearArmory(string what, string where1, string where2="");
    //! get worn item
    Item& getArmory(string where);
    //! remove worn armory
    void removeArmory(string what);
    //! writes item data
    void write(ofstream& out) const;
    //! reads item data
    void read(ifstream& in);
    //! get items of the inventory
    inline vector<Item> getItems() {return items_;}
    //! convert inventory to string
    string toString();
    //! convert string to inventory
    void fromString(string it);
    //! delete used items and restructure data
    void compactify();
    //! give back used spells
    void restoreSpells();
    //! render inventory
    void render();
  private:
    //! the lookup table
    map<string,int> lookup_;
    //! the items
    vector<Item> items_;
    //the mount points for armory
    int left_hand_;
    int right_hand_;
    int head_;
    int body_;
    int belt_;
};

#endif
