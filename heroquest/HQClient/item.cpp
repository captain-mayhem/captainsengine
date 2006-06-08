#include "script.h"
#ifdef _CLIENT_
#include "gamestate.h"
#else
#include "../server/gamestate.hh"
#include "../server/admin.hh"
#include "opcodes.hh"
#include "world.hh"
#endif
#include "item.h"
#include "console.h"

Item::Item(){
  count_ = 1;
  id_ = -1;
}

Item::Item(const Item& it){
  name_ = string(it.name_);
  additional_ = string(it.additional_);
  id_ = it.id_;
  count_ = it.count_;
  type_ = it.type_;
  description_ = it.description_;
}

Item Item::operator=(Item it){
  name_ = string(it.name_);
  additional_ = string(it.additional_);
  id_ = it.id_;
  count_ = it.count_;
  type_ = it.type_;
  description_ = it.description_;
  return *this;
}

Item::Item(string name, short id, Type typ, string additional, string description){
  name_ = name;
  additional_ = additional;
  id_ = id;
  type_ = typ;
  count_ = 1;
  description_ = description;
}

bool Item::use(Vector2D source, Vector2D target){
  switch(type_){
    case Spell:{
#ifdef _CLIENT_
      scr.spell(source, target, id_);
      count_--;
#else
      Creature* self = dynamic_cast<Creature*>(wrld.getObject(source));
      string name = self->getPlayer();
      ServerSocket* ss = globl.getPlayer(name)->getSock();
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        return false;
      }
      if (game.isActionPerformed()){
        *ss << toStr(NO_ACTION);
        return false;
      }
      game.setActionPlace(target);
      GameObject* o = wrld.getObject(target);
      bool object = true;
      Creature* toWhich = NULL;
      if (o == NULL)
        object = false;
      else
        toWhich = dynamic_cast<Creature*>(o);
      
      if (self->getBackPointer()){
        *ss << toStr(CHAT)+" You stay on no valid position.";
        return false;
      }
      if (scr.spell(source,target,id_)){
        game.performAction();
        count_--;
        *ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
      }
      else{
        if (object)
          *ss << toStr(CHAT)+" You cannot cast a spell on "+toWhich->getName();
        else
          *ss << toStr(CHAT)+" You cannot cast a spell on Nobody";
        return false;
      }
#endif
      return true;
    }

    case Potion:{
#ifdef _CLIENT_
      scr.item(source, target, id_);
      count_--;
#else
      Creature* self = dynamic_cast<Creature*>(wrld.getObject(source));
      string name = self->getPlayer();
      ServerSocket* ss = globl.getPlayer(name)->getSock();
      if (self->getBackPointer()){
        *ss << toStr(CHAT)+" You stay on no valid position.";
        return false;
      }
      if (scr.item(source, target, id_)){
        count_--;
        *ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
      }
      else{
        *ss << toStr(CHAT)+" You cannot use this item.";
        return true;
      }
#endif
      return true;
    }

    case Armory:{
#ifdef _CLIENT_
      scr.armory(source, target, id_);
#else
      Creature* self = dynamic_cast<Creature*>(wrld.getObject(source));
      string name = self->getPlayer();
      ServerSocket* ss = globl.getPlayer(name)->getSock();
      if (self->getBackPointer()){
        *ss << toStr(CHAT)+" You stay on no valid position.";
        return false;
      }
      if (!scr.armory(source, target, id_)){
        *ss << toStr(CHAT)+" Conflicting armory already in use. Take off the armory that blocks the required body part.";
        return true;
      }
#endif
      return true;
    }

    case Unusable:{
#ifndef _CLIENT_
      Creature* self = dynamic_cast<Creature*>(wrld.getObject(source));
      string name = self->getPlayer();
      ServerSocket* ss = globl.getPlayer(name)->getSock();
      *ss << toStr(CHAT)+" You cannot use this item.";
#endif
      return true;
    }
               
    default:
      return false;
  }
}

//! writes door data
void Item::write(ofstream& out)const{
  short length = name_.size();
  out.write((char*)&length,sizeof(length));
  out.write(name_.c_str(), length);
  length = additional_.size();
  out.write((char*)&length,sizeof(length));
  out.write(additional_.c_str(), length);
  out.write((char*)&type_, sizeof(type_));
  out.write((char*)&id_, sizeof(id_));
  out.write((char*)&count_, sizeof(count_));
  length = description_.size();
  out.write((char*)&length,sizeof(length));
  out.write(description_.c_str(), length);
}

//! reads door data
void Item::read(ifstream& in){
  short length;
  char buffer[4096];
  in.read((char*)&length,sizeof(length));
  in.read(buffer, length);
  buffer[length] = '\0';
  name_ = string(buffer);
  in.read((char*)&length,sizeof(length));
  in.read(buffer, length);
  buffer[length] = '\0';
  additional_ = string(buffer);
  in.read((char*)&type_, sizeof(type_));
  in.read((char*)&id_, sizeof(id_));
  in.read((char*)&count_, sizeof(count_));
  in.read((char*)&length,sizeof(length));
  in.read(buffer, length);
  buffer[length] = '\0';
  description_ = string(buffer);
}
    
string Item::toString(){
  string ret;
  ret += name_+"*";
  ret += additional_+"*";
  ret += toStr(type_)+"*";
  ret += toStr(id_)+"*";
  ret += toStr(count_)+"*";
  string desc = description_;
  
  unsigned int pos = 0;
  unsigned int found;
  while((found = desc.find(" ", pos)) !=string::npos){
    desc[found] = '*';
    pos = found+1;
  }
  ret += desc;
  return ret;
}
    
void Item::fromString(string it){
  unsigned int pos = 0;
  unsigned int found;
  found = it.find('*',pos);
  name_ = it.substr(pos,found-pos);
  pos = found+1;
  found = it.find('*',pos);
  additional_ = it.substr(pos,found-pos);
  pos = found+1;
  found = it.find('*',pos);
  type_ = (Type)toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('*',pos);
  id_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  found = it.find('*',pos);
  count_ = toInt(it.substr(pos,found-pos));
  pos = found+1;
  description_ = it.substr(pos, it.size()-1);
  pos = 0;
  found = 0;
  while((found = description_.find("*", pos)) !=string::npos){
    description_[found] = ' ';
    pos = found+1;
  }
}
