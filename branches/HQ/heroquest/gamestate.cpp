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

#include "common.hh"
#include "camera.hh"
#include "world.hh"
#include "renderer.hh"
#include "texture.hh"
#include "font.hh"
#include "message.hh"
#include "console.hh"
#include "script.hh"
#include "templates.hh"
#include "player.hh"
#include "gamestate.hh"

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
  void (Message::*p)(const char*);
  p = &Message::process;
  consol.setCbFunc(p);
}

//start game
bool GameState::start(){
  //init camera
  cam.positionCamera(Vector3D(-10, 8, -12), Vector3D(-9, 8, -12), Vector3D(0, 1, 0));
  cam.setCameraRadius(1);
 
  //start screen with GUI
  output.glPrint(120, 450, "Server:", 1, HUGE_VAL);
  InputField* in = new InputField();
  in->setPosition(Vector2D(200, 450));
  in->setText(msg.getSetting(0));
  gl->addInputListener(in);
  
  output.glPrint(120, 400, "Port:", 1, HUGE_VAL);
  InputField* in2 = new InputField();
  in2->setPosition(Vector2D(200, 400));
  in2->setText(msg.getSetting(1));
  gl->addInputListener(in2);

  Button* but = new Button();
  but->setPosition(Vector2D(200,300));
  but->setText("   Connect");
  gl->addButtonListener(but);
  void (Renderer::*p)();
  p = &Renderer::connect;
  but->setCbFunc(p);
  
  return true;
}

//run one step further
void GameState::run(){
  consol.update();
  cam.update();
  gl->handleKeys();
  //gl->calculateFrameRate();
}


// ends the game
void GameState::end(){
  status_ = INIT;
  wrld.unload();
  //wrld.deInit();
  cam.positionCamera(Vector3D(-10, 8, -12), Vector3D(-9, 8, -12), Vector3D(0, 1, 0));
  gl->setViewTo3D(false);
  //re-setup GUI to choose level
	gl->clearListeners(false);
	//only the player with admin status can create games
	if (plyr.getStatus() == 2){
		output.glPrint(120, 450, "Package:",1, (float)HUGE_VAL);
		Button* but = new Button();
		but->setPosition(Vector2D(200,450));
		but->setText("basic");
		void (Renderer::*p)();
		p = &Renderer::package;
		but->setCbFunc(p);
		gl->addButtonListener(but,false);

		output.glPrint(120, 400, "Level:", 1, (float)HUGE_VAL);
		but = new Button();
		but->setPosition(Vector2D(200,400));
		but->setText("maze");
		p = &Renderer::level;
		but->setCbFunc(p);
		gl->addButtonListener(but,false);

		but = new Button();
		but->setPosition(Vector2D(200,300));
		but->setText("     Load");
		p = &Renderer::loadLevel;
		but->setCbFunc(p);
		gl->addButtonListener(but,false);
	}
}
