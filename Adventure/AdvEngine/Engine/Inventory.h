#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <list>
#include <map>
#include "AdvDoc.h"
#include "SaveStateProvider.h"

namespace adv{

class ItemObject;
class ExecutionContext;
class Object2D;

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
  void load(const SaveStateProvider::SaveInventory& inv);
  void setCurrent(int invNum) {mCurrentInv = invNum;}
  unsigned getNumItemSlots() {return (unsigned)mInventory[mCurrentInv].size();}
  void realize();
protected:
  void clear();
  typedef std::list<ItemObject*> SingleInv;
  std::map<int, SingleInv> mInventory;
  int mCurrentInv;
  bool mRealized;
};

class InventoryDisplay{
public:
  InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale, int spacing, int depth);
  ~InventoryDisplay();
  void render(Inventory* inv);
  Object2D* getObjectAt(const Vec2i& pos, Inventory* inv);
  void setPosition(const Vec2i& pos);
  Vec2i getPosition();
  void addScrollOffset(int offset, int maxItems);
protected:
  Vec2i mPos;
  Vec2i mSize;
  Vec2f mScale;
  int mSpacing;
  int mItemOffset;
  int mDepth;
};

}

#endif
