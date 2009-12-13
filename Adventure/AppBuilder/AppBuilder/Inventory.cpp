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
      delete *iter;
      mInventory[invnumber].erase(iter);
      break;
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

void Inventory::save(SaveStateProvider::SaveInventory& inv) const{
  for (std::map<int, SingleInv>::const_iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::const_iterator inviter = iter->second.begin(); inviter != iter->second.end(); ++inviter){
      inv.items[iter->first].push_back((*inviter)->getName());
    }
  }
}

////////////////////////////////////////////

InventoryDisplay::InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale)
: mPos(pos+Vec2i(16,16)), mSize(size), mScale(scale), mItemOffset(0){

}

InventoryDisplay::~InventoryDisplay(){

}

void InventoryDisplay::render(Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[1];
  Vec2i pos = mPos;
  int precount = 0;
  int count = 0;
  int invitemwidth = 64;
  int invitemheight = 48;
  for (std::list<Object2D*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    if (precount < mItemOffset){
      ++precount;
      continue;
    }
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

void InventoryDisplay::setPosition(const Vec2i& pos){
  mPos = pos+Vec2i(16,16);
}

Vec2i InventoryDisplay::getPosition(){
  return mPos-Vec2i(16,16);
}

void InventoryDisplay::addScrollOffset(int offset){
  mItemOffset += offset;
  if (mItemOffset < 0)
    mItemOffset = 0;
}