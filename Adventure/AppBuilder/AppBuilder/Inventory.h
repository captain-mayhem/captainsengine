#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <list>
#include <map>

class Object2D;

class Inventory{
public:
  Inventory();
  ~Inventory();
  void addItem(Object2D* item, int invnumber);
protected:
  typedef std::list<Object2D*> SingleInv;
  std::map<int, SingleInv> mInventory;
};

#endif
