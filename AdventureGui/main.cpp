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

using std::cerr;
using namespace Graphics;
using namespace System;

Application* Application::instance_ = NULL;

void Application::init(){
  instance_ = new Application();
}

void Application::_rendererInit(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setClearColor(Vector3D(0.5,0.5,0.5));
  rend->renderMode(Graphics::Filled);
  rend->ortho(1024, 768);
  setupMainGui();
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
  but->setText(" Gehen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(260, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Oeffnen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(380, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Nehmen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(500, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Benutzen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(620, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Ziehen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(740, 100));
  but->setSpan(Vector2D(100,20));
  but->setText(" Schlafen");
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
  but->setText(" Ansehen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(260, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Schliessen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(380, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Reden");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(500, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Kombinieren");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(620, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Druecken");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(740, 50));
  but->setSpan(Vector2D(100,20));
  but->setText(" Lernen");
  System::Engine::instance()->addGuiListener(but);

  but = new Gui::Button();
  but->setPosition(Vector2D(860, 50));
  but->setSpan(Vector2D(100,20));
  but->setText("Charakter");
  System::Engine::instance()->addGuiListener(but);
}

void engineMain(int argc, char** argv){
  Application::instance()->init();
  
  //render callbacks
  Renderer* rend = Engine::instance()->getRenderer();
  //rend->setRenderCB(Application::rendererInit);
  rend->setInitCB(Application::rendererInit);

  //input callbacks
  //Input::Keyboard::instance()->setKeyDownCB(Editor::keypress);
  //Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  //Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);
}

