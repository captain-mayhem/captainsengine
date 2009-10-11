#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <list>
#include <map>
#include "AdvDoc.h"

class Object2D;

class Inventory{
  friend class InventoryDisplay;
public:
  Inventory();
  ~Inventory();
  void addItem(Object2D* item, int invnumber);
  void removeItem(const std::string& item, int invnumber);
  Object2D* getItem(const std::string& name);
protected:
  typedef std::list<Object2D*> SingleInv;
  std::map<int, SingleInv> mInventory;
};

class InventoryDisplay{
public:
  InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale);
  ~InventoryDisplay();
  void render(Inventory* inv);
  Object2D* getObjectAt(const Vec2i& pos, Inventory* inv);
protected:
  Vec2i mPos;
  Vec2i mSize;
  Vec2f mScale;
};

#endif
