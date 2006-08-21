//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: player.cpp                                                 |
//  ==================================================================

#include "assert.h" 
#include <iostream>
#include <fstream>
#include <sstream>
#include "clientsocket.h"
#include "common.h"
#include "opcodes.h"
#include "world.h"
#include "gui/console.h"
#include "message.h"
#include "templates.h"
#include "trade.h"
#include "player.h"

using std::cout;
using std::endl;
using std::cin;
using std::cerr;
using std::ios;
using std::ifstream;
using std::ostringstream;

#define line *System::Engine::instance()->getFont()

extern ClientSocket* sock;
extern string path;
extern string home;

//Csontructor: set initial values
Player::Player(): status_(0), zargon_(false){
  activeCreature_ = "nobody";
  turn_ = false;
  wonLevel_ = 0;
  wonPackage_ = 0;
  trade_ = NULL;
}

Player::Player(const Player& p){
  name_ = string(p.name_);
  status_ = p.status_;
  zargon_ = p.zargon_;
  activeCreature_ = string(p.activeCreature_);
  turn_ = p.turn_;
  wonLevel_ = p.wonLevel_;
  wonPackage_ = p.wonPackage_;
  trade_ = new Trade(*p.trade_);
}

//Destructor
Player::~Player(){
  SAFE_DELETE(trade_);
}

//login on server
void Player::login(const string& name){
  if (status_ != 0){
    consol << "You are already logged in.\n";
    return;
  }
  
  consol << "Enter password: ";
  consol.setParams(Vector3D(), Graphics::Color(0,0,0,0), 0);
  void (*p)(const string&, int, void*);
  p = &Message::special;
  consol.setSpecialFunc(p, LOGIN, NULL);
  name_ = name;
}

//add a hero from the console
void Player::addHero(const string& filename, const ClientSocket* ss){
  Hero* heroe = new Hero();
  if(!heroe->load(filename)){
    consol << "Hero not found or wrong version";
    delete heroe;
    return;
  }
  bool found = false;
  //Already in use?
  for (unsigned i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getHeros()[i].getName() == heroe->getName() && wrld.getHeros()[i].getPlayer() == heroe->getPlayer()){
      consol << "Hero is already in use.";
      delete heroe;
      return;
    }
  }
  //get free starting positions
  for (unsigned i = 0; i < wrld.getHeroSize(); i++){
    //position already used
    if (wrld.getStarts()[i] == Vector2D(-1,-1)){
      continue;
    }
    found = true;
    ostringstream tmp;
    tmp << i << ": x->" <<  wrld.getStarts()[i].x << " y->" << wrld.getStarts()[i].y;
    consol << tmp;
  }
  if (!found){
    consol << "All starting positions occupied. You cannot participate in this game";
    delete heroe;
    return;
  }
  consol << "Choose starting position:";
  void (*p)(const string&, int, void*);
  p = &Message::special;
  consol.setSpecialFunc(p, PLAY, (void*)heroe);
}

//add a hero from the GUI
Hero* Player::addHero(const string& filename){
  Hero* heroe = new Hero();
  if(!heroe->load(filename)){
    line << "Hero not found or wrong version";
    delete heroe;
    return NULL;
  }
  bool found = false;
  //Already in use?
  for (unsigned i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getHeros()[i].getName() == heroe->getName() && wrld.getHeros()[i].getPlayer() == heroe->getPlayer()){
      line << "Hero is already in use.";
      delete heroe;
      return NULL;
    }
  }
  //get free starting positions
  for (unsigned i = 0; i < wrld.getHeroSize(); i++){
    //position already used
    if (wrld.getStarts()[i] == Vector2D(-1,-1)){
      continue;
    }
    found = true;
  }
  if (!found){
    line << "All starting positions occupied. You cannot participate in this game";
    delete heroe;
    return NULL;
  }
  return heroe;
}

//play Zargon
void Player::addZargon(){
  zargon_ = true;
}

//isZargon
bool Player::isZargon() const {
  return zargon_;
}

//find the currently controlled creature
Creature* Player::getCreature() const {
  Creature* ret = NULL;
  Hero* heros = wrld.getHeros();
  for (int i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getStarts()[i] != Vector2D(-1,-1))
      continue;
    Hero h = heros[i];
    if (h.getName() == activeCreature_){
      ret = &heros[i];
      return ret;
    }
  }
  vector<Monster*>& monsters = wrld.getMonsters();
  for (unsigned i = 0; i < monsters.size(); i++){
    Monster* m = monsters[i];
    if (m->getName() == activeCreature_){
      ret = monsters[i];
      return ret;
    }
  }
  return ret;
}

void Player::loadStatus(){
  string path = home+name_+".sav";
  ifstream in(path.c_str(), ios::binary);
  if (!in){
    //the login is used the first time
    //write standard file
    ofstream out(path.c_str(), ios::binary);
    out.write((char*)&wonPackage_, sizeof(wonPackage_));
    out.write((char*)&wonLevel_, sizeof(wonLevel_));
    out.close();
  }
  else{
    in.read((char*)&wonPackage_, sizeof(wonPackage_));
    in.read((char*)&wonLevel_, sizeof(wonLevel_));
    in.close();
  }
  //setup trading
  //open appropriate trade file
  trade_ = new Trade();
  string pth = "data/trade"+toStr(wonPackage_)+".dat";
  ifstream trade(pth.c_str());
  if (!trade)
    cerr << pth << " not found!";
  int numits;
  trade >> numits;
  string name;
  for (int i = 0; i < numits; i++){
    trade >> name;
    Item it = Templates::instance()->searchItem(name);
    trade_->addItem(it);
  }
  trade.close();
}
