#ifndef TRADE_H
#define TRADE_H

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#ifdef _CLIENT_
#include "renderer/renderer.h"
#include "renderer/font.h"
#endif
#include "item.h"

using std::map;
using std::vector;
using std::ifstream;
using std::ofstream;

class Trade{
  public:
    //! Constructor
    Trade();
    //! Copy Constructor
    Trade(const Trade& i);
    //! Destuctor
    ~Trade() {}
    //! add item to inventory
    void addItem(Item it);
    //! delete item from inventory
    bool deleteItem(string name);
    //! get item from inventory
    Item getItem(const string name);
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
