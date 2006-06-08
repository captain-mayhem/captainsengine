//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: hero.cpp                                                   |
//  ==================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#ifdef _CLIENT_
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "console.h"
#include "common.h"
#include "renderer.h"
#include "camera.h"
#include "texture.hh"
#endif
#include "opcodes.h"
#include "world.h"
#include "player.h"
#include "hero.h"

using std::cout;
using std::cin;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::ostringstream;
using std::ios;

extern string path;

//CONSTRUCTOR
Hero::Hero() : money_(0), spellClasses_(0) {
  Creature();
  diceToMove_ = true;
}

Hero::Hero(const Hero& h){
  position_ = h.position_;
  id_ = h.id_;
  active_ = h.active_;
  interpolate_ = h.interpolate_;

  name_ = string(h.name_);
  type_ = string(h.type_);
  player_ = string(h.player_);
  movement_ = h.movement_;
  attack_ = h.attack_;
  defend_ = h.defend_;
  body_ = h.body_;
  maxBody_ = h.maxBody_;
  mind_ = h.mind_;
  maxMind_ = h.maxMind_;
  bp_ = h.bp_;
  actPos_ = h.actPos_;
  positions_ = queue<Vector3D>(h.positions_);
	lookAt_ = h.lookAt_;
	xRot_ = h.xRot_;
  items_ = Inventory(h.items_);
  sleeping_ = h.sleeping_;
  diceToMove_ = h.diceToMove_;
  
  money_ = h.money_;
  spellClasses_ = h.spellClasses_;

}

//DESTRUCTOR
Hero::~Hero(){
}

// Constructor that inits all values
Hero::Hero(const string& player, const string& name, const string& type, const char body, const char mind, const char attack, const char defend, const int money, const char movement, const short id, const Vector2D& position, short spellClasses){
  player_ = player;
  name_ = name;
  type_ = type;
  body_ = body;
  maxBody_ = body;
  mind_ = mind;
  maxMind_ = mind;
  attack_ = attack;
  defend_ = defend;
  id_ = id;
  position_ = position;
  money_ = money;
  movement_ = movement;
  sleeping_ = false;
  spellClasses_ = spellClasses_;

  diceToMove_ = true;
}

//create a hero
void Hero::Create(const string& path, const vector<Hero>& heros, const string& playerName){
#ifdef _CLIENT_
  //write available heros to the console
  ostringstream tmp;
  for (unsigned i = 0; i < heros.size(); i++){
    Hero heroe = heros[i];
    tmp << i << ": " << heroe.getType();
    tmp << " - Body points: " << heroe.getBody();
    tmp << ", Attack: " << heroe.getAttack();
    tmp << ", Defence: " << heroe.getDefence();
    tmp << ", Intelligence: " << heroe.getMind();
    tmp << ", Movement dice: " << heroe.getMovement();
    tmp << ", Spell types: " << heroe.getSpellClasses();
    tmp << "\n";
  }
  tmp << "Choose your character class: ";
  consol << tmp;
  //set the create function
  void (Message::*p)(const string&, int, void*);
  p = &Message::special;
  consol.setSpecialFunc(p, CREATE, NULL); 
#endif
}

//write hero to disk
void Hero::write(const string& p) const{
  ofstream out(p.c_str(), ios::binary);

  float version = VERSION;
  out.write((char*)&version, sizeof(version));

  short length = player_.length();
  out.write((char*)&length, sizeof(length));
  out.write(player_.c_str(), length);

  length = name_.length();
  out.write((char*)&length, sizeof(length));
  out.write(name_.c_str(), length);

  length = type_.length();
  out.write((char*)&length, sizeof(length));
  out.write(type_.c_str(), length);

  out.write(&attack_, sizeof(attack_));
  out.write(&defend_, sizeof(defend_));
  out.write(&maxBody_, sizeof(maxBody_));
  out.write(&maxMind_, sizeof(maxMind_));
  out.write((char*)&money_, sizeof(money_));
  out.write(&movement_, sizeof(movement_));

  out.write((char*)&id_, sizeof(id_));
  out.write((char*)&spellClasses_, sizeof(spellClasses_));

  items_.write(out);
  
  out.close();
}

//makes a string out of all creature values
const string Hero::data(short posIdx){
  string data = player_+" "+name_+" "+type_;
  data += " "+toStr(body_)+" "+toStr(mind_)+" "+toStr(attack_);
  data += " "+toStr(defend_);
  data += " "+toStr(position_.x)+" "+toStr(position_.y)+" "+toStr(id_);
  data += " "+toStr(posIdx);
  data += " "+toStr(movement_);
  data += " "+toStr(money_);
  data += " "+items_.toString();
  data += " "+toStr(spellClasses_);
  return data;
}


bool Hero::load(const string& filename){
  float version;
  ifstream in(filename.c_str(), ios::binary);

  in.read((char*)&version, sizeof(version));
  if (version < VERSION || version > VERSION+0.01){
    in.close();
    return false;
  }

  short length;
  char buffer[256];
  
  in.read((char*)&length, sizeof(length));
  in.read(buffer, length);
  buffer[length] = '\0';
  string player = string(buffer);
  
  in.read((char*)&length, sizeof(length));
  in.read(buffer, length);
  buffer[length] = '\0';
  string name = string(buffer);
  
  in.read((char*)&length, sizeof(length));
  in.read(buffer, length);
  buffer[length] = '\0';
  string type = string(buffer);
  
  in.read(&attack_, sizeof(attack_));
  in.read(&defend_, sizeof(defend_));
  in.read(&maxBody_, sizeof(maxBody_));
  body_ = maxBody_;
  in.read(&maxMind_, sizeof(maxMind_));
  mind_ = maxMind_;
  in.read((char*)&money_, sizeof(money_));
  in.read(&movement_, sizeof(movement_));

  in.read((char*)&id_, sizeof(id_));
  in.read((char*)&spellClasses_, sizeof(spellClasses_));
  
  items_.read(in);
  
  in.close();

  setPlayer(player);
  setName(name);
  setType(type);

  return true;
}


void Hero::render() const {
#ifdef _CLIENT_
  glBindTexture(GL_TEXTURE_2D, tex.heroTex[id_]);
  Creature::render();
#endif
}


void Hero::render2D() const {
#ifdef _CLIENT_
  glBindTexture(GL_TEXTURE_2D, tex.heroTex[id_]);
  Creature::render2D();
#endif
}


//! let the hero win the game
void Hero::win(){
  //restore spells
  items_.restoreSpells();
  //delete used items
  items_.compactify();
  //restore body and mind
  body_ = maxBody_;
  mind_ = maxMind_;
  //deactivate and save hero
  active_ = false;
  wrld.removeObject(position_);
  string filename = path+"/data/"+player_+name_+".sav";
  write(filename);
}