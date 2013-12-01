#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <list>
#include <map>
#include "AdvDoc.h"
#include "SaveStateProvider.h"

namespace adv{

class ItemObject;

class Inventory{
  friend class InventoryDisplay;
public:
  Inventory();
  ~Inventory();
  void addItem(ItemObject* item, int invnumber);
  void removeItem(const std::string& item, int invnumber, ExecutionContext* del_script);
  ItemObject* getItem(const std::string& name);
  ItemObject* getItem(const std::string& name, int invnumber);
  void save(SaveStateProvider::SaveInventory& inv) const;
  void setCurrent(int invNum) {mCurrentInv = invNum;}
protected:
  typedef std::list<ItemObject*> SingleInv;
  std::map<int, SingleInv> mInventory;
  int mCurrentInv;
};

class InventoryDisplay{
public:
  InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale, int depth);
  ~InventoryDisplay();
  void render(Inventory* inv);
  Object2D* getObjectAt(const Vec2i& pos, Inventory* inv);
  void setPosition(const Vec2i& pos);
  Vec2i getPosition();
  void addScrollOffset(int offset);
protected:
  Vec2i mPos;
  Vec2i mSize;
  Vec2f mScale;
  int mItemOffset;
  int mDepth;
};

}

#endif
