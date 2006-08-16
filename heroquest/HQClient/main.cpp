#include <iostream>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "gamestate.h"
#include "camera.h"
#include "world.h"
//#include "console.h"
#include "player.h"
#include "message.h"
#include "script.h"
#include "clientsocket.h"
#include "input/keyboard.h"
#include "input/mouse.h"

#include "renderer.h"

using namespace System;
using namespace Graphics;

GameState game;
Camera cam;
World wrld;
Player plyr;
Message msg;
Script scr;
ClientSocket* sock;
string path;
string home;

void engineMain(int argc, char** argv){
  path = "";
  home = "";
#ifdef WIN32
  char* drv = getenv("HOMEDRIVE");
  char* pth = getenv("HOMEPATH");
  if (drv && pth)
    home = string(drv)+pth;
  else
    home = path;
#endif
#ifdef UNIX
  char* pth = getenv("HOME");
  if (pth)
    home = string(pth)+"/";
  else
    home = path;
#endif
  System::Log << home;
  Renderer* rend = Engine::instance()->getRenderer();
  HQRenderer::init(rend);

  rend->setInitCB(HQRenderer::initialize);
  rend->setRenderCB(HQRenderer::paint);
  rend->setResizeCB(HQRenderer::resize);
  Input::Keyboard::instance()->setKeyDownCB(HQRenderer::special);
  Input::Keyboard::instance()->setKeyUpCB(HQRenderer::keyUp);
  Input::Keyboard::instance()->setAsciiCB(HQRenderer::ascii);
  Input::Mouse::instance()->setButtonDownCB(HQRenderer::buttonDown);
  Input::Mouse::instance()->setMouseMoveCB(HQRenderer::mouseMove);
}
