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
using Graphics::Font;

//Constructor
GameState::GameState() : moves_(0){
}

//Destructor
GameState::~GameState(){
}

//init game
void GameState::init(){
  status_ = INIT;
  Templates::init();
  msg.init();
  wrld.init();
  scr.init();
  void (*p)(const char*);
  p = &Message::process;
  consol.setCbFunc(p);
}

//start game
bool GameState::start(){
  //init camera
  cam.positionCamera(Vector3D(-10, 8, -12), Vector3D(-9, 8, -12), Vector3D(0, 1, 0));
  cam.setCameraRadius(1);
  
  //start screen with GUI
  Font* f = System::Engine::instance()->getFont(0);
  f->setColor(1.0,1.0,1.0);
  f->glPrint(120, 450, "Server:", 1, HUGE_VAL);
  InputField* in = new InputField();
  in->setPosition(Vector2D(200, 450));
  in->setText(msg.getSetting(0));
  System::Engine::instance()->addInputListener(in);
  
  f->glPrint(120, 400, "Port:", 1, HUGE_VAL);
  InputField* in2 = new InputField();
  in2->setPosition(Vector2D(200, 400));
  in2->setText(msg.getSetting(1));
  System::Engine::instance()->addInputListener(in2);

  Button* but = new Button();
  but->setPosition(Vector2D(200,300));
  but->setText("   Connect");
  System::Engine::instance()->addButtonListener(but);
  //void (*p)();
  //p = &Renderer::connect;
  but->setCbFunc(Menu::connect);
  
  return true;
}

//run one step further
void GameState::run(){
  //consol.update();
  cam.update();
  //gl->handleKeys();
  //gl->calculateFrameRate();
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
  Font* f = System::Engine::instance()->getFont(0);
  System::Engine::instance()->clearListeners(false);
	//only the player with admin status can create games
	if (plyr.getStatus() == 2){
    f->setColor(1,1,1);
		f->glPrint(120, 450, "Package:",1, (float)HUGE_VAL);
		Button* but = new Button();
		but->setPosition(Vector2D(200,450));
		but->setText("basic");
		//void (Renderer::*p)();
		//p = &Renderer::package;
		but->setCbFunc(Menu::package);
    System::Engine::instance()->addButtonListener(but,false);
    
		f->glPrint(120, 400, "Level:", 1, (float)HUGE_VAL);
		but = new Button();
		but->setPosition(Vector2D(200,400));
		but->setText("maze");
		//p = &Renderer::level;
		but->setCbFunc(Menu::level);
    System::Engine::instance()->addButtonListener(but,false);

		but = new Button();
		but->setPosition(Vector2D(200,300));
		but->setText("     Load");
		//p = &Renderer::loadLevel;
		but->setCbFunc(Menu::loadLevel);
    System::Engine::instance()->addButtonListener(but,false);
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
  ifstream in("levels/levels.dat");
  int number;
  in >> number;
  string name;
  if (!in){
    System::Log << "levels.dat (packages) not found.";
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
    System::Log << "levels.dat (levels) not found.";
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
