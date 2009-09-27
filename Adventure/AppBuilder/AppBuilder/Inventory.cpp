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