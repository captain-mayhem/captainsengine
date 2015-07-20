#include "system/engine.h"
#include "renderer/renderer.h"
#include "window/window.h"
#include "system/utilities.h"
#include "system/script.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "io/Tracing.h"
#include "io/TraceManager.h"
#include "window/splashwindow.h"

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"
#include "CmdReceiver.h"

using namespace adv;

TR_CHANNEL(ADV_CGE_Window);

std::string filename;
std::string savegame;
AdvDocument* adoc = NULL;
CommandReceiver receiver;
bool openConsole;
CGE::SplashWindow* splashWindow = NULL;

Vec2i windowsize(640,480);

void quit();
void set_mouse(int x, int y);
void splash_screen(unsigned width, unsigned height, unsigned numChannels, void* data);

void fileChanged(std::string const& file){
  TR_USE(ADV_CGE_Window);
  TR_INFO("File %s written", file.c_str());
}

void init(){
  TR_USE(ADV_CGE_Window);
  Engine::init();
  adoc = new AdvDocument();
  //adoc->getProjectSettings()->savedir = "C:\\tmp";
  adoc->setFileChangedCB(fileChanged);
  adoc->setSpashScreenCB(splash_screen);
  if (!adoc->loadDocument(filename)){
    TR_ERROR("Cannot load %s", filename.c_str());
    CGE::Engine::instance()->requestShutdown();
    return;
  }
  windowsize = adoc->getProjectSettings()->resolution;
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  //rend->getWindow()->setTitle(adoc->getProjectSettings()->)
  rend->getWindow()->changeSize(windowsize.x, windowsize.y);
  
  Engine::instance()->setData(adoc);
  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  AdvRenderer::init();

  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, (float)windowsize.x, (float)windowsize.y, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  GL()matrixMode(MM_MODELVIEW);
  GL()color4ub(255,255,255,255);

  glDisable(GL_DEPTH_TEST);
  GL()enableClientState(ATTR_VERTEX_ARRAY);
  GL()enableClientState(ATTR_TEXTURE_COORD_ARRAY);
  GL()enable(GL_TEXTURE_2D);
  //glAlphaFunc(GL_GREATER, 0);
  //glEnable(GL_ALPHA_TEST);
  //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glViewport(0, 0, windowsize.x, windowsize.y);
  
  if (CGE::Script::instance()->getBoolSetting("debugPorts"))
    receiver.start();
  Engine::instance()->initGame(quit, set_mouse);
  delete splashWindow;
  splashWindow = NULL;
  rend->getWindow()->show(true);
}

void deinit(){
  Engine::instance()->exitGame();
  if (CGE::Script::instance()->getBoolSetting("debugPorts"))
    receiver.stop();

  AdvRenderer::deinit();

  SoundEngine::deinit();
  Engine::deinit();
  delete adoc;
}

void quit(){
  CGE::Engine::instance()->requestShutdown();
}

void set_mouse(int x, int y){
  Input::Mouse::instance()->setMousePos(x, y);
}

void splash_screen(unsigned width, unsigned height, unsigned numChannels, void* data){
  splashWindow = new CGE::SplashWindow(CGE::Engine::instance()->getWindow());
  CGE::Image image;
  image.setFormat(numChannels, width, height);
  image.setData((unsigned char*)data);
  splashWindow->show(image);
  image.setData(NULL);
}

void render(){
  double start = getTime();
  //CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, (float)windowsize.x, (float)windowsize.y, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  GL()matrixMode(MM_MODELVIEW);
  glDisable(GL_DEPTH_TEST);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glClearColor(0.0,0.0,0.0,1.0);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()loadIdentity();

  receiver.processCommands();
  unsigned time = (unsigned)(CGE::Engine::instance()->getFrameInterval()*1000);
  Engine::instance()->render(time);

  SoundEngine::instance()->update(time);

  double diff = getTime()-start;
  int sleeptime = 5 - (int)(diff*1000);
  if (sleeptime < 0)
    sleeptime = 0;

  CGE::Thread::sleep(sleeptime); //just a little sleep to avoid frame times < 5ms.

  if (!savegame.empty()){
    savegame = Engine::instance()->getSaver()->saveSlotToPath(atoi(savegame.c_str()));
    Engine::instance()->getSaver()->load(savegame);
    savegame.clear();
  }
}

void mouse_move(int x, int y, int button){
  if (x >= 0 && x <= windowsize.x && y >= 0 && y <= windowsize.y)
    Engine::instance()->setCursorPos(Vec2i(x,y));
}

void mouse_click(int x, int y, int button){
  Vec2i pos((int)(x/(float)SCREENWIDTH*windowsize.x), (int)(y/(float)SCREENHEIGHT*windowsize.y));
  if (button == MB_LEFT)
    Engine::instance()->leftClick(pos);
  else if (button == MB_RIGHT)
    Engine::instance()->rightClick(pos);
}

void mouse_release(int x, int y, int button){
  Vec2i pos((int)(x/(float)SCREENWIDTH*windowsize.x), (int)(y/(float)SCREENHEIGHT*windowsize.y));
  if (button == MB_LEFT)
    Engine::instance()->leftRelease(pos);
}

void double_click(int x, int y, int button){
  Vec2i pos((int)(x/(float)SCREENWIDTH*windowsize.x), (int)(y/(float)SCREENHEIGHT*windowsize.y));
  Engine::instance()->doubleClick(pos);
}

void mouse_wheel(int x, int y, int delta){
  Engine::instance()->mouseWheel(delta);
}

void key_press(int key){
  Engine::instance()->keyPress(key);
}

void key_release(int key){
  Engine::instance()->keyRelease(key);
}

void key_ascii(unsigned char ascii){
  Engine::instance()->keyAscii((char)ascii);
}

void cleanup(){
#ifdef WIN32
  if (openConsole){
    fclose(stdout);
    fclose(stderr);
    FreeConsole();
  }
#endif
}

void engineMain(int argc, char** argv){
  atexit(cleanup);
#ifdef WIN32
  openConsole = CGE::Script::instance()->getBoolSetting("openConsole");
  if (openConsole){
    AllocConsole();
    freopen("CONOUT$", "wb", stdout);
    freopen("CONOUT$", "wb", stderr);
  }
#endif
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  if (argc > 1)
    filename = argv[1];
  else
    filename = "data/game.dat";
  if (argc > 2)
    savegame = argv[2];
  else
    savegame = "";
  CGE::Utilities::replaceWith(filename, '\\', '/');
  //render callbacks
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->setRenderCB(render);
  rend->setInitCB(init);
  rend->setDeinitCB(deinit);

  //input callbacks
  Input::Keyboard::instance()->setKeyDownCB(key_press);
  Input::Keyboard::instance()->setKeyUpCB(key_release);
  Input::Keyboard::instance()->setAsciiCB(key_ascii);
  Input::Mouse::instance()->setButtonDownCB(mouse_click);
  Input::Mouse::instance()->setButtonUpCB(mouse_release);
  Input::Mouse::instance()->setMouseMoveCB(mouse_move);
  Input::Mouse::instance()->setDoubleClickCB(double_click);
  Input::Mouse::instance()->setWheelTurnCB(mouse_wheel);
  Input::Mouse::instance()->showCursor(false);
}
