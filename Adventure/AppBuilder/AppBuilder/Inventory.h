#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <list>
#include <map>
#include "AdvDoc.h"
#include "SaveStateProvider.h"

namespace adv{

class Object2D;

class Inventory{
  friend class InventoryDisplay;
public:
  Inventory();
  ~Inventory();
  void addItem(Object2D* item, int invnumber);
  void removeItem(const std::string& item, int invnumber, ExecutionContext* del_script);
  Object2D* getItem(const std::string& name);
  void save(SaveStateProvider::SaveInventory& inv) const;
protected:
  typedef std::list<Object2D*> SingleInv;
  std::map<int, SingleInv> mInventory;
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
