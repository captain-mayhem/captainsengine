#ifndef INVENTORY_HH
#define INVENTORY_HH

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
//#include "array.hh"
#ifdef _CLIENT_
#include "renderer/renderer.h"
//#include "renderer/vertexbuffer.h"
#include "renderer/font.h"
#endif
#include "item.h"

using std::map;
using std::vector;
using std::ifstream;
using std::ofstream;

//using Graphics::Font;

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
    //! delete item savely
    //! if the item is worn, it is taken off before deleting the item
    bool deleteItemSavely(const Vector2D& pos, string name);
    //! get item from inventory
    Item getItem(const string name);
    //! use item from inventory
    bool useItem(string name, Math::Vector2D pos, Math::Vector2D target);
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
    //! update inventory
    void update();
#ifdef _CLIENT_
    //! get chosen item
    inline Item* getChosenItem() {return chosenItem_;}
    //! deselect item
    inline void deselect() {chosenItem_ = NULL;}
    //! is visible
    inline void makeVisible(bool doit) {visible_ = doit;}
#endif
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
    int breast_;
#ifdef _CLIENT_
    //! the font
    ::Graphics::Font* fnt_;
    //! the item page to be displayed
    unsigned short page_;
    //! the chosen item
    Item* chosenItem_;
    //! if inventory is visible
    bool visible_;
#endif
};

#endif
