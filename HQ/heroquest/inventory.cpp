#include <iostream>
#include <GL/gl.h>
#include "texture.hh"
#include "inventory.hh"
#ifdef _CLIENT_
#include "player.hh"
#include "renderer.hh"
#endif

using std::cerr;

Inventory::Inventory(){
  items_.push_back(Item());
  left_hand_ = 0;
  right_hand_ = 0;
  head_ = 0;
  body_ = 0;
  belt_ = 0;
#ifdef _CLIENT_
  fnt_.setBase(text.getBase());
  fnt_.setColor(0,1,1);
  page_ = 1;
  chosenItem_ = NULL;
  visible_ = true;
#endif
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
  if (!visible_)
    return;
  //background
  glDisable(GL_BLEND);
  glColor4f(1.0,1.0,1.0,1.0);
  glBindTexture(GL_TEXTURE_2D, tex.otherTex[1]);
  glBegin(GL_QUADS);
    glTexCoord2f(1, 0);
    glVertex2i(SCREENWIDTH, SCREENHEIGHT);
    glTexCoord2f(0, 0);
    glVertex2i(0, SCREENHEIGHT);
    glTexCoord2f(0, 1);
    glVertex2i(0, 0);
    glTexCoord2f(1, 1);
    glVertex2i(SCREENWIDTH, 0);
  glEnd();
  glEnable(GL_BLEND);

  //items
  Vector2D pos(50, 485);
  short count = 0;
  for (unsigned i = 9*(page_-1)+1; i < items_.size() && i < 9*page_+1; i++){
    string name = items_[i].getName();
    if (name.size() > 11)
      name.erase(11);
    int fill = (11 - name.size())/2;
    name.insert(name.begin(), fill, ' ');
    fnt_.glPrint(pos.x, pos.y, name.c_str(), 1);
    pos.x += 148;
    count++;
    if (count == 3){
      pos.x = 50;
      count = 0;
      pos.y -= 193;
    }
  }
  fnt_.glPrint(177, 50, ("<<  Page "+toStr(page_)+"  >>").c_str(), 0);

  //stats
  Creature* c = plyr.getCreature();
  fnt_.glPrint(600, 700, ("Name: "+c->getName()).c_str(), 0);
  fnt_.glPrint(800, 700, ("Race: "+c->getType()).c_str(), 0);
  fnt_.glPrint(600, 680, ("Attack: "+toStr(c->getAttack())).c_str(), 0);
  fnt_.glPrint(800, 680, ("Defense: "+toStr(c->getDefence())).c_str(), 0);
  fnt_.glPrint(600, 660, ("Body: "+toStr(c->getBody())+"/"+toStr(c->getMaxBody())).c_str(), 0);
  fnt_.glPrint(800, 660, ("Mind: "+toStr(c->getMind())+"/"+toStr(c->getMaxMind())).c_str(), 0);
  Hero* h = dynamic_cast<Hero*>(c);
  if (h){
    fnt_.glPrint(600, 620, ("Money: "+toStr(h->getMoney())).c_str(), 0);
  }

  //worn items
  Item ite = getArmory("right hand");
  if (ite.isValid()){
    fnt_.glPrint(600, 580, (ite.getName()+" is held in the right hand").c_str(), 0);
  }
  ite = getArmory("left hand");
  if (ite.isValid()){
    fnt_.glPrint(600, 560, (ite.getName()+" is held in the left hand").c_str(), 0);
  }
  ite = getArmory("head");
  if (ite.isValid()){
    fnt_.glPrint(600, 540, (ite.getName()+" is worn on the head").c_str(), 0);
  }
  ite = getArmory("body");
  if (ite.isValid()){
    fnt_.glPrint(600, 520, (ite.getName()+" is worn on the body").c_str(), 0);
  }
  ite = getArmory("belt");
  if (ite.isValid()){
    fnt_.glPrint(600, 500, (ite.getName()+" is worn on the belt").c_str(), 0);
  }

  Vector2D click = gl->getMousePos();
  fnt_.glPrint(600, 460, (toStr(click.x)+"/"+toStr(click.y)).c_str(), 0);

  if (chosenItem_ != NULL){
    fnt_.glPrint(600, 440, chosenItem_->getName().c_str(), 0);
  }

  fnt_.render();
#endif
}

void Inventory::update(){
#ifdef _CLIENT_
  if (!visible_)
    return;
  Vector2D click = gl->getMousePos();
  //forward button
  if (click.x >= 300 && click.x <= 325){
    if (click.y >= 700 && click.y <= 720){
      gl->resetMousePos();
      page_++;
    }
  }
  //backward button
  if (click.x >= 180 && click.x <= 200){
    if (click.y >= 700 && click.y <= 720){
      if (page_ > 1){
        gl->resetMousePos();
        page_--;
      }
    }
  }
  //first item column
  if (click.x >= 65 && click.x <= 155){
    if (click.y >= 125 && click.y <= 280){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+1;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+4;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+7;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
  }
  //second item column
  if (click.x >= 210 && click.x <= 300){
    if (click.y >= 125 && click.y <= 280){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+2;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+5;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+8;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
  }
  //third item column
  if (click.x >= 355 && click.x <= 450){
    if (click.y >= 125 && click.y <= 280){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+3;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 315 && click.y <= 475){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+6;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
    else if (click.y >= 510 && click.y <= 670){
      gl->resetMousePos();
      unsigned idx = 9*(page_-1)+9;
      if (idx < items_.size())
        chosenItem_ = &items_[idx];
    }
  }
#endif
}
