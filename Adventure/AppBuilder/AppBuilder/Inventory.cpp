#include "Inventory.h"
#include "EngineObjects.h"

Inventory::Inventory(){

}

Inventory::~Inventory(){
  for (std::map<int, SingleInv>::iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    while (!iter->second.empty()){
      delete iter->second.front();
      iter->second.pop_front();
    }
  }
}

void Inventory::addItem(Object2D* item, int invnumber){
  mInventory[invnumber].push_back(item);
}

void Inventory::removeItem(const std::string& item, int invnumber){
  for (SingleInv::iterator iter = mInventory[invnumber].begin(); iter != mInventory[invnumber].end(); ++iter){
    if ((*iter)->getName() == item){
      mInventory[invnumber].erase(iter);
    }
  }
}

Object2D* Inventory::getItem(const std::string& name){
  for (std::map<int, SingleInv>::iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2){
      if ((*iter2)->getName() == name){
        return *iter2;
      }
    }
  }
  return NULL;
}

////////////////////////////////////////////

InventoryDisplay::InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale)
: mPos(pos), mSize(size), mScale(scale){

}

InventoryDisplay::~InventoryDisplay(){

}

void InventoryDisplay::render(Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[1];
  Vec2i pos = mPos;
  int count = 0;
  int invitemwidth = 100;
  int invitemheight = 70;
  for (std::list<Object2D*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    (*iter)->setPosition(pos);
    (*iter)->render();
    ++count;
    int xpos = count % mSize.x;
    pos.x = mPos.x+xpos*invitemwidth;
    int ypos = count / mSize.x;
    pos.y = mPos.y+ypos*invitemheight;
    if (count > mSize.x*mSize.y){
      break;
    }
  }
}

Object2D* InventoryDisplay::getObjectAt(const Vec2i& pos, Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[1];
  for (std::list<Object2D*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    if ((*iter)->isHit(pos))
      return (*iter);
  }
  return NULL;
}
