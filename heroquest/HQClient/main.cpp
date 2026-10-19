#include <iostream>
#include "system/engine.h"
#include "system/application.h"
#include "platform/userdirs.h"
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
#include "io/Tracing.h"
#include "io/TraceManager.h"

#include "renderer.h"

using namespace CGE;

GameState game;
::Camera cam;
World wrld;
Player plyr;
Message msg;
Script scr;
ClientSocket* sock;
string path;
string home;

TR_CHANNEL(HQ_Client)

class HQClientApp : public CGE::IApplication{
public:
  void init(int argc, char** argv) override;
  void render() override {HQRenderer::paint();}
  void onResize(int width, int height) override {HQRenderer::resize(width, height);}
  void onKeyDown(int key) override {HQRenderer::special(key);}
  void onKeyUp(int key) override {HQRenderer::keyUp(key);}
  void onChar(unsigned char ch) override {HQRenderer::ascii(ch);}
  void onMouseDown(int x, int y, int button) override {HQRenderer::buttonDown(x, y, button);}
  void onMouseMove(int x, int y, int buttons) override {HQRenderer::mouseMove(x, y, buttons);}
};

void HQClientApp::init(int argc, char** argv){
  //the GUI and the 2D map are laid out for 1024x768
  CGE::Engine::instance()->getVirtualScreen().setSize(1024, 768);
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  TR_USE(HQ_Client);
  path = "";
  //the heroes' saves; %APPDATA%\heroquest on Windows, as before
  CGE::UserDirs::setAppName("heroquest");
  home = CGE::UserDirs::getDataDir() + "/";
  TR_DEBUG("Home is: %s", home.c_str());
  Renderer* rend = Engine::instance()->getRenderer();
  HQRenderer::init(rend);
  HQRenderer::initialize();
}

CGE_MAIN(HQClientApp)
