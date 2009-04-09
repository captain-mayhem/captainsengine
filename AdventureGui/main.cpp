#include <iostream>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"
#include "math/vector.h"
#include "gui/messagebox.h"
#include "gui/dropdown.h"
#include "input/keyboard.h"
#include "input/mouse.h"

#include "main.h"

#include "AdventureCore.h"
#include "SQL.h"

using std::cerr;
using namespace CGE;

Application* Application::instance_ = NULL;

void Application::init(){
  instance_ = new Application();
}

void Application::gotoButton(Gui::GuiElement* elem){
  instance_->core()->showWays();
}

Application::Application(){
}

Application::~Application(){
  delete core_;
  delete sql_;
}

void Application::_rendererInit(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setClearColor(Vector3D(0.0,0.0,0.0));
  rend->renderMode(CGE::Filled);
  rend->ortho(SCREENWIDTH, SCREENHEIGHT);

  sql_ = new SQL("C:\\Dokumente und Einstellungen\\Weller\\Eigene Dateien\\adventure.hac");
  //sql_->execute("SELECT curr_room FROM characters WHERE chid=0;");
  //int rid = sql_->getResultString("curr_room", 0);
  core_ = new AdventureCore(sql_);

  setupMainGui();
}

void Application::_render(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
}

void Application::_mouseDown(int x, int y, int button){
  //calculate line
  if (x < SECOND_CURSOR_X)
    return;
  int lineidx = (SECOND_CURSOR_Y-(SCREENHEIGHT-LINE_SPACING-y))/LINE_SPACING;
  if (lineidx < 0)
    return;
  core_->performAction(lineidx);
}

void Application::setupMainGui(){
  Renderer* rend = Engine::instance()->getRenderer();
  Gui::Button* but = new Gui::Button();
  but->setPosition(Vector2D(20, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Spiel");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(140, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Gehe");
  but->setCbFunc(gotoButton);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(260, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Oeffne");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(380, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Nimm");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(500, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Benutze");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(620, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Ziehe");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(740, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Schlafe");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(860, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Inventar");
  CGE::Engine::instance()->addGuiListener(but);

  //second row
  but = new Gui::Button();
  but->setPosition(Vector2D(20, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Karte");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(140, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Sieh an");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(260, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Schliesse");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(380, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Rede");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(500, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Kombiniere");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(620, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Druecke");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(740, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Lerne");
  CGE::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(860, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Charakter");
  CGE::Engine::instance()->addGuiListener(but);

  //TODO make it correct
  std::string chid_ = "0";
  std::string rid = sql_->execute("SELECT curr_room FROM characters where chid="+chid_+";");
  core_->gotoRoom(rid);
}

void engineMain(int argc, char** argv){
  Application::instance()->init();
  
  //render callbacks
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setInitCB(Application::rendererInit);
  rend->setRenderCB(Application::render);

  //input callbacks
  //Input::Keyboard::instance()->setKeyDownCB(Editor::keypress);
  //Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Application::mouseDown);
}

