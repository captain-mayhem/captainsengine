//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Stefan Ruff                                                      |
// | sisnruff@stud.informatik.uni-erlangen.de                         |
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
#include "console.hh"
#include "common.hh"
#include "renderer.hh"
#include "camera.hh"
#include "texture.hh"
#endif
#include "opcodes.hh"
#include "world.hh"
#include "player.hh"
#include "monster.hh"

using std::cout;
using std::cin;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::ostringstream;
using std::ios;


//CONSTRUCTOR
Monster::Monster() : canMove_(false){
  Creature();
  diceToMove_ = false;
}

Monster::Monster(const Monster& m){
  position_ = m.position_;
  id_ = m.id_;
  active_ = m.active_;
  interpolate_ = m.interpolate_;

  name_ = string(m.name_);
  type_ = string(m.type_);
  player_ = string(m.player_);
  movement_ = m.movement_;
  attack_ = m.attack_;
  defend_ = m.defend_;
  body_ = m.body_;
  maxBody_ = m.maxBody_;
  mind_ = m.mind_;
  maxMind_ = m.maxMind_;
  bp_ = m.bp_;
  actPos_ = m.actPos_;
  positions_ = queue<Vector3D>(m.positions_);
	lookAt_ = m.lookAt_;
	xRot_ = m.xRot_;
  items_ = Inventory(m.items_);
  sleeping_ = m.sleeping_;
  diceToMove_ = m.diceToMove_;
  
  canMove_ = m.canMove_;
	brev_ = string(m.brev_);
}

//DESTRUCTOR
Monster::~Monster(){
}

// Constructor that inits all values
Monster::Monster(const string& player, const string& name, const string& type, 
								 const char body, const char mind, const char attack, 
								 const char defend, const char movement, const short id, 
								 const Vector2D& position){
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
  movement_ = movement;
	canMove_ = false;
  sleeping_ = false;
}

void Monster::render() const {
#ifdef _CLIENT_
  glBindTexture(GL_TEXTURE_2D, tex.monsterTex[id_]);
  Creature::render();
#endif
}

void Monster::render2D() const {
#ifdef _CLIENT_
  glBindTexture(GL_TEXTURE_2D, tex.monsterTex[id_]);
  Creature::render2D();
#endif
}

