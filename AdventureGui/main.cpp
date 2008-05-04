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
using namespace Graphics;
using namespace System;

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
  rend->renderMode(Graphics::Filled);
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
  int lineidx = (SECOND_CURSOR_Y-(SCREENHEIGHT-y))/LINE_SPACING;
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
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(140, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Gehe");
  but->setCbFunc(gotoButton);
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(260, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Oeffne");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(380, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Nimm");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(500, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Benutze");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(620, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Ziehe");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(740, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Schlafe");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(860, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Inventar");
  System::Engine::instance()->addGuiListener(but);

  //second row
  but = new Gui::Button();
  but->setPosition(Vector2D(20, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Karte");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(140, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Sieh an");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(260, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Schliesse");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(380, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Rede");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(500, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Kombiniere");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(620, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Druecke");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(740, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Lerne");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(860, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Charakter");
  System::Engine::instance()->addGuiListener(but);

  //TODO make it correct
  core_->displayText("Ich muss hier einen langen Text schreiben, der weit über eine Zeile geht, um die Funktionsweise des Textdisplays zu erkunden.");
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

