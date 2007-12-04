#include <iostream>
#include <fstream>
#include <string>
#include "common.h"
#include "mesh/model.h"
#include "mesh/mesh.h"
#include "templates.h"

using std::ifstream;
using std::string;

extern string path;

Templates* Templates::templ = NULL;

Templates::Templates(){
}

Templates::~Templates(){
}

void Templates::load(){
  string temp;
  string file;
  
  //read items
  file = path + "../HQClient/data/items.dat";
  ifstream in(file.c_str());
  if (!in){
    cerr << "Could not load items.dat";
  }
  while (in >> temp){
    Item ite;
    ite.setName(temp);
    in >> temp; in >> temp; in >> temp;
    ite.setId((short)toInt(temp));
    in >> temp; in >> temp;
    if (temp == "potion")
      ite.setType(Item::Potion);
    else if (temp == "unusable")
      ite.setType(Item::Unusable);
    else if (temp == "artifact")
      ite.setType(Item::Artifact);
    in >> temp; in >> temp;
    string desc;
    while(temp != "/description"){
      desc += temp;
      desc +=" ";
      in >> temp;
    }
    ite.setDescription(desc);
    items_.push_back(ite);
  }
  in.close();

  //read armory
  file = path + "../HQClient/data/armory.dat";
  ifstream in2(file.c_str());
  if (!in2){
    cerr << "Could not load armory.dat";
  }
  while (in2 >> temp){
    Item ite;
    ite.setType(Item::Armory);
    ite.setName(temp);
    in2 >> temp; in2 >> temp; in2 >> temp;
    ite.setId((short)toInt(temp));
    in2 >> temp; in2 >> temp;
    ite.setAdditional(temp);
    in2 >> temp; in2 >> temp;
    string desc;
    while(temp != "/description"){
      desc += temp;
      desc +=" ";
      in2 >> temp;
    }
    ite.setDescription(desc);
    armory_.push_back(ite);
  }
  in2.close();

  //read monsters
  file = path + "../HQClient/data/monsters.dat";
	ifstream in3(file.c_str());
  if (!in3){
    cerr << "Could not load monsters.dat";
  }
	while(in3 >> temp){
		Monster m;
		m.setType(temp);
		in3 >> temp; in3 >> temp; in3 >> temp;
		m.setId(toInt(temp));
		in3 >> temp; in3 >> temp;
    if (temp.size() < 2)
      temp += " ";
		m.setBrev(temp);
		in3 >> temp; in3 >> temp;
		m.setMovement(toInt(temp));
		in3 >> temp; in3 >> temp;
		m.setAttack(toInt(temp));
		in3 >> temp; in3 >> temp;
		m.setDefence(toInt(temp));
		in3 >> temp; in3 >> temp;
		m.setBody(toInt(temp));
		m.setMaxBody(toInt(temp));
		in3 >> temp; in3 >> temp;
		m.setMind(toInt(temp));
		m.setMaxMind(toInt(temp));
		monsterTypes_.push_back(m);
	}
	in3.close();
    
  //read spells
  file = path + "../HQClient/data/spells.dat";
  ifstream in4(file.c_str());
  if (!in4){
    cerr << "Could not load spells.dat";
  }
  while (in4 >> temp){
    Item spell;
    spell.setType(Item::Spell);
    spell.setName(temp);
    in4 >> temp; in4 >> temp; in4 >> temp;
    spell.setId((short)toInt(temp));
    in4 >> temp; in4 >> temp;
    spell.setAdditional(temp);
    in4 >> temp; in4 >> temp;
    string desc;
    while(temp != "/description"){
      desc += temp;
      desc +=" ";
      in4 >> temp;
    }
    spell.setDescription(desc);
    spells_.push_back(spell);
  }
  in4.close();
  
  //read overlays
  file = path + "../HQClient/data/overlays.dat";
  ifstream in5(file.c_str());
  if (!in5){
    cerr << "Could not load overlays.dat";
  }
  while (in5 >> temp){
    Overlay* o = new Overlay();
    o->setName(temp);
    in5 >> temp; in5 >> temp; in5 >> temp;
    o->setId((short)toInt(temp));
    in5 >> temp; in5 >> temp;
    o->setShort(temp);
    in5 >> temp; in5 >> temp;
    o->setWidth((short)toInt(temp));
    in5 >> temp; in5 >> temp;
    o->setHeight((short)toInt(temp));
    in5 >> temp; in5 >> temp;
    if (temp == "yes")
      o->setWalkable(true);
    else
      o->setWalkable(false);
    in5 >> temp; in5 >> temp;
    string desc;
    while(temp != "/description"){
      desc += temp;
      desc +=" ";
      in5 >> temp;
    }
    o->setDescription(desc);
    overlays_.push_back(o);
  }
  in5.close();
 
  //read models
  file = path + "../HQClient/models/models.dat";
  ifstream in6(file.c_str());
  if (!in6){
    cerr << "Could not load models.dat";
  }
  string name;
  int idx;
  int number;
  in6 >> temp;
  in6 >> number;
  for (int i = 0; i < number; i++){
    in6 >> name;
    in6 >> idx;
    MeshGeo::Mesh* msh = new MeshGeo::Mesh();
#ifdef _CLIENT_
    msh->loadFromFile("../HQClient/models/"+name);
#endif
    MeshGeo::Model* mod = new MeshGeo::Model(msh);
    models_.push_back(mod);
  }
  in6.close();
}

Item Templates::getItem(string name){
  vector<Item>::iterator it = items_.begin();
  while( it != items_.end() ) {
    if (it->getName() == name)
      return *it;
    it++;
  }
  return Item("",-1,Item::NoItem);
}

vector<Item> Templates::getItems(Item::Type type){
  vector<Item> result;
  vector<Item>::iterator it = items_.begin();
  while( it != items_.end() ) {
    if (it->getType() == type)
      result.push_back(*it);
    it++;
  }
  return result;
}

Item Templates::getArmory(string name){
  vector<Item>::iterator it = armory_.begin();
  while( it != armory_.end() ) {
    if (it->getName() == name)
      return *it;
    it++;
  }
  return Item("",-1,Item::NoItem);
}

Monster Templates::getMonster(string name){
  vector<Monster>::iterator it = monsterTypes_.begin();
  while( it != monsterTypes_.end() ) {
    if (it->getType() == name)
      return *it;
    it++;
  }
  return Monster();
}

Item Templates::getSpell(string name){
  vector<Item>::iterator it = spells_.begin();
  while( it != spells_.end() ) {
    if (it->getName() == name)
      return *it;
    it++;
  }
  return Item("",-1,Item::NoItem);
}

vector<Item> Templates::getSpells(string type){
  vector<Item> result;
  vector<Item>::iterator it = spells_.begin();
  while( it != spells_.end() ) {
    if (it->getAdditional() == type)
      result.push_back(*it);
    it++;
  }
  return result;
}

Item Templates::searchItem(string name){
  Item result;
  result = getItem(name);
  if(result.getType() != Item::NoItem)
    return result;
  result = getArmory(name);
  if(result.getType() != Item::NoItem)
    return result;
  result = getSpell(name);
  if(result.getType() != Item::NoItem)
    return result;
  return Item("",-1,Item::NoItem);
}

Overlay* Templates::getOverlay(string brev){
  for(unsigned int i=0; i<overlays_.size(); i++){
    if((overlays_[i])->getShort() == brev){
      return overlays_[i];
    }
  }
  return NULL;
}
