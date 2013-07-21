#include "Inventory.h"
#include "EngineObjects.h"
#include "Engine.h"

using namespace adv;

Inventory::Inventory() : mCurrentInv(1){

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
  mInventory[invnumber].push_front(item);
}

void Inventory::removeItem(const std::string& item, int invnumber, ExecutionContext* del_script){
  for (SingleInv::iterator iter = mInventory[invnumber].begin(); iter != mInventory[invnumber].end(); ++iter){
    if (_stricmp((*iter)->getName().c_str(), item.c_str()) == 0){
      if (del_script == (*iter)->getScript()){
        //do not delete as it is currently running
        (*iter)->setScript(NULL);
        del_script->setOwner(NULL);
        del_script->setExecuteOnce();
      }
      delete *iter;
      mInventory[invnumber].erase(iter);
      break;
    }
  }
}

Object2D* Inventory::getItem(const std::string& name){
  for (std::map<int, SingleInv>::iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2){
      if (_stricmp((*iter2)->getName().c_str(), name.c_str()) == 0){
        return *iter2;
      }
    }
  }
  return NULL;
}

void Inventory::save(SaveStateProvider::SaveInventory& inv) const{
  for (std::map<int, SingleInv>::const_iterator iter = mInventory.begin(); iter != mInventory.end(); ++iter){
    for (SingleInv::const_reverse_iterator inviter = iter->second.rbegin(); inviter != iter->second.rend(); ++inviter){
      inv.items[iter->first].push_back((*inviter)->getName());
    }
  }
  inv.current = mCurrentInv;
}

////////////////////////////////////////////

InventoryDisplay::InventoryDisplay(const Vec2i& pos, const Vec2i& size, const Vec2f& scale, int depth)
: mPos(pos+Vec2i(32,8)), mSize(size), mScale(scale), mItemOffset(0), mDepth(depth){

}

InventoryDisplay::~InventoryDisplay(){

}

void InventoryDisplay::render(Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[inv->mCurrentInv];
  Vec2i pos = mPos;
  int precount = 0;
  int count = 0;
  int invitemwidth = 56;
  int invitemheight = 48;
  for (std::list<Object2D*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    if (precount < mItemOffset){
      ++precount;
      continue;
    }
    (*iter)->setPosition(pos);
    int olddepth = (*iter)->getAnimation()->getDepth();
    (*iter)->render();
    if (mDepth > olddepth)
      (*iter)->setDepth(mDepth);
    ++count;
    int xpos = count % mSize.x;
    pos.x = mPos.x+xpos*invitemwidth;
    int ypos = count / mSize.x;
    pos.y = mPos.y+ypos*invitemheight;
    if (count >= mSize.x*mSize.y){
      break;
    }
  }
}

Object2D* InventoryDisplay::getObjectAt(const Vec2i& pos, Inventory* inv){
  Inventory::SingleInv tmp = inv->mInventory[inv->mCurrentInv];
  int precount = 0;
  int count = 0;
  for (std::list<Object2D*>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter){
    if (precount < mItemOffset){
      ++precount;
      continue;
    }
    if ((*iter)->isHit(pos))
      return (*iter);
    ++count;
    if (count >= mSize.x*mSize.y)
      break;
  }
  return NULL;
}

void InventoryDisplay::setPosition(const Vec2i& pos){
  mPos = pos+Vec2i(32,8);
}

Vec2i InventoryDisplay::getPosition(){
  return mPos-Vec2i(32,8);
}

void InventoryDisplay::addScrollOffset(int offset){
  mItemOffset += offset;
  if (mItemOffset < 0)
    mItemOffset = 0;
}
