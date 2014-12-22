//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: gamestate.cpp                                              |
//  ==================================================================

#include "system/engine.h"

#include "common.h"
#include "camera.h"
#include "world.h"
#include "renderer.h"
#include "textureManager.h"
#include "renderer/font.h"
#include "gui/gui.h"
#include "gui/dropdown.h"
#include "message.h"
#include "gui/console.h"
#include "script.h"
#include "templates.h"
#include "player.h"
#include "menu.h"
#include "gamestate.h"

using Gui::InputField;
using Gui::Button;
using Gui::DropDownButton;
using CGE::Font;

TR_CHANNEL(HQ_Client_Gamestate);

//Constructor
GameState::GameState() : moves_(0){
}

//Destructor
GameState::~GameState(){
}

//init game
bool GameState::init(){
  status_ = INIT;
  Templates::init();
  msg.init();
  if (!wrld.init())
    return false;
  scr.init();
  void (*p)(const char*);
  p = &Message::process;
  consol.setCbFunc(p);
  return true;
}

//start game
bool GameState::start(){
  //init camera
  cam.positionCamera(Vector3D(-10, 8, -12), Vector3D(-9, 8, -12), Vector3D(0, 1, 0));
  cam.setCameraRadius(1);
  
  //start screen with GUI
  Font* f = CGE::Engine::instance()->getFont(0);
  f->setColor(1.0,1.0,1.0);
  f->print(120, 450, "Server:", 1, (float)HUGE_VAL);
  InputField* in = new InputField();
  in->setPosition(Vector2D(200, 450));
  in->setText(msg.getSetting(0));
  CGE::Engine::instance()->addGuiListener(in);
  
  f->print(120, 400, "Port:", 1, (float)HUGE_VAL);
  InputField* in2 = new InputField();
  in2->setPosition(Vector2D(200, 400));
  in2->setText(msg.getSetting(1));
  CGE::Engine::instance()->addGuiListener(in2);

  Button* but = new Button();
  but->setPosition(Vector2D(200,300));
  but->setText("   Connect");
  CGE::Engine::instance()->addGuiListener(but);
  //void (*p)();
  //p = &Renderer::connect;
  but->setCbFunc(Menu::connect);
  
  return true;
}

//run one step further
void GameState::run(){
  msg.process();
  cam.update();
}


// ends the game
void GameState::end(){
  //HQRenderer::instance()->setViewTo3D(false);
  plyr.saveStatus();
  plyr.resetZargon();
  cam.positionCamera(Vector3D(-10, 8, -12), Vector3D(-9, 8, -12), Vector3D(0, 1, 0));
  status_ = INIT;
  wrld.unload();
  //wrld.deInit();
  //re-setup GUI to choose level
  Font* f = CGE::Engine::instance()->getFont(0);
  CGE::Engine::instance()->clearListeners(false);
	//only the player with admin status can create games
	if (plyr.getStatus() == 2){
    f->setColor(1,1,1);
    f->print(120, 450, "Package:",1, (float)HUGE_VAL);
	  DropDownButton* but = new DropDownButton();
		but->setPosition(Vector2D(200,450));
    but->calcDDPos(1);
		but->setText("basic");
    but->setCbFunc(Menu::level);
    CGE::Engine::instance()->addGuiListener(but,false);

		f->print(120, 400, "Level:", 1, (float)HUGE_VAL);
		DropDownButton* but2 = new DropDownButton();
		but2->setPosition(Vector2D(200,400));
    but2->calcDDPos(1);
    CGE::Engine::instance()->addGuiListener(but2,false);
    game.choosePackage(but, but2);

		Button* but3 = new Button();
		but3->setPosition(Vector2D(200,300));
		but3->setText("     Load");
		but3->setCbFunc(Menu::loadLevel);
    CGE::Engine::instance()->addGuiListener(but3,false);
	}
}

Vector2D GameState::getNextCreaturePos(){
  //get current creature
  Creature* c = plyr.getCreature();
  if (!c){
    //TODO return any creature
    cerr << "not set";
    return Vector2D(-1,-1);
  }
  
  bool currFound = false;
  Hero* heros = wrld.getHeros();
  for (int i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getStarts()[i] != Vector2D(-1,-1))
      continue;
    Hero h = heros[i];
    if (h.getName() == c->getName() && h.getPlayer() == c->getPlayer()){
      //found current
      currFound = true;
      continue;
    }
    if (currFound && h.getPlayer() == plyr.getName()){
      //found next
      return h.getPosition();
    }
  }
  vector<Monster*>& monsters = wrld.getMonsters();
  for (unsigned i = 0; i < monsters.size(); i++){
    Monster* m = monsters[i];
    if (m->getName() == c->getName() && m->getPlayer() == c->getPlayer()){
      //found current
      currFound = true;
      continue;
    }
    if (currFound && m->getPlayer() == plyr.getName()){
      //found next
      return m->getPosition();
    }
  }
  for (int i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getStarts()[i] != Vector2D(-1,-1))
      continue;
    Hero h = heros[i];
    if (h.getName() == c->getName() && h.getPlayer() == c->getPlayer()){
      //found current
      currFound = true;
      continue;
    }
    if (currFound && h.getPlayer() == plyr.getName()){
      //found next
      return h.getPosition();
    }
  }
  for (unsigned i = 0; i < monsters.size(); i++){
    Monster* m = monsters[i];
    if (m->getName() == c->getName() && m->getPlayer() == c->getPlayer()){
      //found current
      currFound = true;
      continue;
    }
    if (currFound && m->getPlayer() == plyr.getName()){
      //found next
      return m->getPosition();
    }
  }
  //nothing suitable found, so the next is the old
  return c->getPosition();
}

void GameState::choosePackage(DropDownButton* pack, DropDownButton* level){
  TR_USE(HQ_Client_Gamestate);
  ifstream in("levels/levels.dat");
  int number;
  in >> number;
  string name;
  if (!in){
    TR_ERROR("levels.dat (packages) not found.");
    return;
  }
  for (int i = 0; i < number; i++){
    in >> name;
    pack->addEntry(name);
    if (i == 0)
      pack->setText(name);
  }
  in.close();

  ifstream in2(string("levels/"+pack->getText()+"/levels.dat").c_str());
  in2 >> number;
  if (!in2){
    TR_ERROR("levels.dat (levels) not found.");
    return;
  }
  for (int i = 0; i < number; i++){
    in2 >> name;
    level->addEntry(name);
    if (i == 0)
      level->setText(name);
  }
  in2.close();
}

// add a random number to repository
void GameState::addRandom(int number){
  randoms_.push(number);
}
