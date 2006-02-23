#include <iostream>
#include "inventory.hh"

using std::cerr;

Inventory::Inventory(){
  items_.push_back(Item());
  left_hand_ = 0;
  right_hand_ = 0;
  head_ = 0;
  body_ = 0;
  belt_ = 0;
}

Inventory::Inventory(const Inventory& i){
  items_ = vector<Item>(i.items_);
  lookup_ = map<string,int>(i.lookup_);
  left_hand_ = i.left_hand_;
  right_hand_ = i.right_hand_;
  head_ = i.head_;
  body_ = i.body_;
  belt_ = i.belt_;
}

void Inventory::addItem(Item it){
  int idx = lookup_[it.getName()];
  //no such item stored
  if(idx == 0){
    items_.push_back(it);
    lookup_[it.getName()] = items_.size()-1;
  }
  else{
    items_[idx].increase();
  }
}

bool Inventory::deleteItem(string name){
  int idx = lookup_[name];
  if (idx != 0){
    //last item deleted
    if (!items_[idx].decrease()){
      items_[idx].reset();
      return false;
    }
    return true;
  }
  return false;
}

bool Inventory::useItem(string name, Vector2D pos, Vector2D target){
  int idx = lookup_[name];
  if (idx == 0)
    return false;
  if(!items_[idx].isValid())
    return false;
  bool success = items_[idx].use(pos, target);
  return success;
}
    
//! writes item data
void Inventory::write(ofstream& out)const{
  short length = items_.size()-1;
  out.write((char*)&length, sizeof(length));
  for (int i = 1; i < length+1; i++){
    items_[i].write(out);
  }
  out.write((char*)&left_hand_, sizeof(left_hand_));
  out.write((char*)&right_hand_, sizeof(right_hand_));
  out.write((char*)&head_, sizeof(head_));
  out.write((char*)&body_, sizeof(body_));
  out.write((char*)&belt_, sizeof(belt_));
}

//! reads item data
void Inventory::read(ifstream& in){
  short length;
  in.read((char*)&length, sizeof(length));
  for (int i = 0; i < length; i++){
    Item it;
    it.read(in);
    items_.push_back(it);
    lookup_[it.getName()] = items_.size()-1;
  }
  in.read((char*)&left_hand_, sizeof(left_hand_));
  in.read((char*)&right_hand_, sizeof(right_hand_));
  in.read((char*)&head_, sizeof(head_));
  in.read((char*)&body_, sizeof(body_));
  in.read((char*)&belt_, sizeof(belt_));
}

string Inventory::toString(){
  string ret = "";
  for (unsigned i = 0; i < items_.size(); i++){
    ret += items_[i].toString()+"!";
  }
  ret += toStr(left_hand_)+"$";
  ret += toStr(right_hand_)+"$";
  ret += toStr(head_)+"$";
  ret += toStr(body_)+"$";
  ret += toStr(belt_)+"$";
  return ret;
}

void Inventory::fromString(string it){
  int pos = 0;
  unsigned found;
  items_.clear();
  while ((found = it.find('!',pos)) != string::npos){
    string tmp = it.substr(pos,found-pos);
    pos = found+1;
    Item ite;
    ite.fromString(tmp);
    items_.push_back(ite);
    lookup_[ite.getName()] = items_.size()-1;
  }
  //find the armory mount points
  string tmp;
  found = it.find('$', pos);
  left_hand_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  right_hand_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  head_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  body_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('$', pos);
  belt_ = toInt(it.substr(pos,found-pos));
  pos = found+1;

}


bool Inventory::wearArmory(string what, string where1, string where2){
  int idx = lookup_[what];
  if (idx == 0)
    return false;
  if(!items_[idx].isValid())
    return false;
  if (where1 == "left hand" || where2 == "left hand"){
    if (left_hand_ != 0)
      return false;
    left_hand_ = idx;
  }
  if (where1 == "right hand" || where2 == "right hand"){
    if (right_hand_ != 0)
      return false; 
    right_hand_ = idx;
  }
  if (where1 == "head" || where2 == "head"){
    if (head_ != 0)
      return false;
    head_ = idx;
  }
  if (where1 == "body" || where2 == "body"){
    if (body_ != 0)
      return false;
    body_ = idx;
  }
  if (where1 == "belt" || where2 == "belt"){
    if (belt_ != 0)
      return false;
    belt_ = idx;
  }
  return true;
}


// get worn item
Item& Inventory::getArmory(string where){
  if (where == "left hand")
    return items_[left_hand_];
  if (where == "right hand")
    return items_[right_hand_];
  if (where == "head")
    return items_[head_];
  if (where == "body")
    return items_[body_];
  if (where == "belt")
    return items_[belt_];
  return items_[0];
}
    
// remove worn armory
void Inventory::removeArmory(string what){
  int idx = lookup_[what];
  if (idx == 0 || !items_[idx].isValid())
    return;
  if (left_hand_ == idx)
    left_hand_ = 0;
  if (right_hand_ == idx)
    right_hand_ = 0;
  if (head_ == idx)
    head_ = 0;
  if (body_ == idx)
    body_ = 0;
  if (belt_ == idx)
    belt_ = 0;
}


//delete used items and restructure data
void Inventory::compactify(){
  vector<Item>::iterator iter = items_.begin();
  iter++;
  int position = 1;
  int offset = 0;
  while (iter != items_.end()){
    Item& it = *iter;
    string name = it.getName();
    lookup_[name] = position-offset;
    if (!it.isValid()){
      offset++;
      iter = items_.erase(iter);
    }
    else{
      iter++;
    }
    position++;
  }
}


// give back used spells
void Inventory::restoreSpells(){
  vector<Item>::iterator iter = items_.begin();
  iter++;
  while (iter != items_.end()){
    Item& it = *iter;
    if (!it.isValid() && it.getType() == Item::Spell){
      //looks like a used spell, so give one more
      it.increase();
    }
    iter++;
  }
}

void Inventory::render(){
#ifdef _CLIENT_
  
#endif
}
