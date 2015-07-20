//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: creature.cpp                                               |
//  ==================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#ifdef _CLIENT_
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "gui/console.h"
#include "renderer.h"
//#include "texture.hh"
#include "player.h"
#include "camera.h"
#include "renderer/forms.h"
#endif
#include "common.h"
#include "opcodes.h"
#include "world.h"
//#include "hero.hh"

using std::cout;
using std::cin;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::ostringstream;
using std::ios;
using namespace CGE;


//CONSTRUCTOR
Creature::Creature() : name_(), type_(){
  this->setActive();
  this->setPosition(Vector2D());
  actPos_ = Vector3D(0,2,0);
  camPos_ = Vector3D(0,2,0);
	lookAt_ = Vector3D(1,2,0);
  xRot_ = 0;
  sleeping_ = false;
  diceToMove_ = false;
  model_ = NULL;
}

Creature::Creature(const Creature& c){
  position_ = c.position_;
  id_ = c.id_;
  active_ = c.active_;
  interpolate_ = c.interpolate_;

  name_ = string(c.name_);
  type_ = string(c.type_);
  player_ = string(c.player_);
  movement_ = c.movement_;
  attack_ = c.attack_;
  defend_ = c.defend_;
  body_ = c.body_;
  maxBody_ = c.maxBody_;
  mind_ = c.mind_;
  maxMind_ = c.maxMind_;
  bp_ = c.bp_;
  actPos_ = c.actPos_;
  positions_ = queue<Vector3D>(c.positions_);
	lookAt_ = c.lookAt_;
	camPos_ = c.camPos_;
	xRot_ = c.xRot_;
  items_ = Inventory(c.items_);
  sleeping_ = c.sleeping_;
  diceToMove_ = c.diceToMove_;
  model_ = c.model_;
}

//DESTRUCTOR
Creature::~Creature(){
}

// Constructor that inits all values
Creature::Creature(const string& player, const string& name, const string& type, const char body, const char mind, const char attack, const char defend, const char movement, const short id, const Vector2D& position){
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
  actPos_ = Vector3D(0,2,0);
  camPos_ = Vector3D(0,2,0);
	lookAt_ = Vector3D(1,2,0);
  xRot_ = 0;
  sleeping_ = false;
  model_ = NULL;
}

//makes a string out of all creature values
const string Creature::data(short posIdx){
  string data = player_+" "+name_+" "+type_;
  data += " "+toStr(body_)+" "+toStr(mind_)+" "+toStr(attack_);
  data += " "+toStr(defend_);
  data += " "+toStr(position_.x)+" "+toStr(position_.y)+" "+toStr(id_);
  data += " "+toStr(posIdx);
  data += " "+toStr(movement_);
  return data;
}

//Render the creature
void Creature::render() const{
#ifdef _CLIENT_
  //don't display yourself when you are the camera
  if (plyr.getName() == player_ && name_ == plyr.getActCreature())
    return;

  if (model_){
    model_->setupMaterial();
    model_->render(CGE::Mesh::DRAW_ALL);
    model_->resetMaterial();
  }
#endif
}

//Render the creature in 2D
void Creature::render2D() const{
#ifdef _CLIENT_	
  int height = wrld.getMapSize().y;
  float dx = (float)(SCREENWIDTH/wrld.getMapSize().x);
  float dy = (float)(SCREENHEIGHT/height);

  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
  
  if (active_)
    rend->setColor(1,1,1,1);
  else
    rend->setColor(1,1,1,0.3f);
  rend->enableTexturing(true);
  
  forms->drawQuad(Vec2f(position_.x*dx, (height - position_.y)*dy - dy), Vec2f(dx, dy));
  
  rend->setColor(1,1,1,1);
  rend->enableTexturing(false);
#endif
}

//! interpolate creature position
void Creature::update(){
#ifdef _CLIENT_
  //no interpolation
  if (!interpolate_){
    actPos_ = wrld.modelToRealPos(position_);
    if (model_){
      Vector3D pos = actPos_;
      pos.y = 0;
      CGE::Matrix mat = Matrix(Matrix::Translation, pos);
      model_->setTrafo(mat);
    }
    while(!positions_.empty())
      positions_.pop();
    return;
  }
  Vector3D toReach = wrld.modelToRealPos(position_);
  //a new position is there
  if (positions_.empty() || toReach != positions_.back()){
    positions_.push(toReach);
  }
  //position reached
  if (actPos_ == positions_.front()){
    positions_.pop();
  }
  if (positions_.empty())
    return;
  toReach = positions_.front();
  // move how far
  float move = (float)(CGE::Engine::instance()->getFrameInterval()*cam.getSpeed());
  // a little epsilon to avoid oscillation around the target point
  float epsilon = 0.1f;
  // move in the correct direction
  if (actPos_.x < toReach.x-epsilon)
    actPos_.x += move;
  else if (actPos_.x > toReach.x+epsilon)
    actPos_.x -= move;
  else if (actPos_.z < toReach.z-epsilon)
    actPos_.z += move;
  else if (actPos_.z > toReach.z+epsilon)
    actPos_.z -= move;
  if (model_){
    Vector3D pos = actPos_;
    pos.y = 0;
    CGE::Matrix mat = Matrix(Matrix::Translation, pos);
    model_->setTrafo(mat);
  }
#endif
}
