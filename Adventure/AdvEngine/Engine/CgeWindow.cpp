#include "system/engine.h"
#include "system/application.h"
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
#include "CmdReceiver.h"

using namespace adv;

TR_CHANNEL(ADV_CGE_Window);

String filename;
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
  //the game and its input use the project resolution
  CGE::Engine::instance()->getVirtualScreen().setSize(windowsize.x, windowsize.y);
  
  Engine::instance()->setData(adoc);
  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  rend->switchMatrixStack(CGE::Projection);
  rend->resetModelView();
  CGE::Engine::instance()->getVirtualScreen().ortho(rend, true);

  rend->switchMatrixStack(CGE::Modelview);
  rend->setColor(1.0f, 1.0f, 1.0f, 1.0f);

  rend->enableDepthTest(false);
  //GL()enableClientState(ATTR_VERTEX_ARRAY);
  //GL()enableClientState(ATTR_TEXTURE_COORD_ARRAY);
  rend->enableTexturing(true);
  //glAlphaFunc(GL_GREATER, 0);
  //glEnable(GL_ALPHA_TEST);
  //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableBlend(true);

  rend->viewport(0, 0, windowsize.x, windowsize.y);
  
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
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->switchMatrixStack(CGE::Projection);
  rend->resetModelView();
  CGE::Engine::instance()->getVirtualScreen().ortho(rend, true);

  rend->switchMatrixStack(CGE::Modelview);
  rend->enableDepthTest(false);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableBlend(true);

  rend->setClearColor(CGE::Vec4f(0.0, 0.0, 0.0, 1.0));
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  rend->resetModelView();

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
    Engine::instance()->getSaver()->load(savegame, false);
    savegame.clear();
  }
}

void mouse_move(int x, int y, int button){
  if (x >= 0 && x <= windowsize.x && y >= 0 && y <= windowsize.y)
    Engine::instance()->setCursorPos(Vec2i(x,y));
}

void mouse_click(int x, int y, int button){
  Vec2i pos(x, y);
  if (button == MB_LEFT)
    Engine::instance()->leftClick(pos);
  else if (button == MB_RIGHT)
    Engine::instance()->rightClick(pos);
}

void mouse_release(int x, int y, int button){
  Vec2i pos(x, y);
  if (button == MB_LEFT)
    Engine::instance()->leftRelease(pos);
}

void double_click(int x, int y, int button){
  mouse_click(x, y, button); //also emit a single click when double click occurs
  Vec2i pos(x, y);
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

class AdventureApp : public CGE::IApplication{
public:
  void init(int argc, char** argv) override;
  void render() override {::render();}
  void shutdown() override {deinit();}
  void onKeyDown(int key) override {key_press(key);}
  void onKeyUp(int key) override {key_release(key);}
  void onChar(unsigned char ch) override {key_ascii(ch);}
  void onMouseDown(int x, int y, int button) override {mouse_click(x, y, button);}
  void onMouseUp(int x, int y, int button) override {mouse_release(x, y, button);}
  void onDoubleClick(int x, int y, int button) override {double_click(x, y, button);}
  void onMouseMove(int x, int y, int buttons) override {mouse_move(x, y, buttons);}
  void onMouseWheel(int x, int y, int delta) override {mouse_wheel(x, y, delta);}
};

void AdventureApp::init(int argc, char** argv){
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
  filename.replaceWith('\\', '/');
  if (!filename.endsWith(".dat") && !filename.endsWith(".adv"))
    filename += "/data/game.dat";
  Input::Keyboard::instance()->enableBuitInConsole(false);
  Input::Mouse::instance()->showCursor(false);
  ::init();
}

CGE_MAIN(AdventureApp)
