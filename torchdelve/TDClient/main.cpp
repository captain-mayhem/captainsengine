#include <iostream>
#include "system/engine.h"
#include "system/application.h"
#include "platform/userdirs.h"
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
#include "window/window.h"

#include "renderer.h"
#include "layout.h"

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

TR_CHANNEL(TD_Client)

class TDClientApp : public CGE::IApplication{
public:
  void init(int argc, char** argv) override;
  void render() override {TDRenderer::paint();}
  void shutdown() override {TDRenderer::releaseResources();}
  void onResize(int width, int height) override {TDRenderer::resize(width, height);}
  void onKeyDown(int key) override {TDRenderer::special(key);}
  void onKeyUp(int key) override {TDRenderer::keyUp(key);}
  //! its keys are ASCII letters
  void onChar(unsigned int ch) override {if (ch < 0x80) TDRenderer::ascii((unsigned char)ch);}
  void onMouseDown(int x, int y, int button) override {TDRenderer::buttonDown(x, y, button);}
  void onMouseMove(int x, int y, int buttons) override {TDRenderer::mouseMove(x, y, buttons);}
};

void TDClientApp::init(int argc, char** argv){
  CGE::Engine::instance()->getWindow()->setTitle("Torchdelve");
  //the GUI and the 2D map are laid out for 1920x1080 (16:9, layout.h)
  CGE::Engine::instance()->getVirtualScreen().setSize(Layout::kScreenWidth, Layout::kScreenHeight);
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  TR_USE(TD_Client);
  path = "";
  //the heroes' saves: %APPDATA%\<appName> on Windows, appName from engine.ini
  //("torchdelve"; a content pack may set its own, tools/install_pack.py)
  home = CGE::UserDirs::getDataDir() + "/";
  TR_DEBUG("Home is: %s", home.c_str());
  TDRenderer::init();
  TDRenderer::initialize();
}

CGE_MAIN(TDClientApp)
