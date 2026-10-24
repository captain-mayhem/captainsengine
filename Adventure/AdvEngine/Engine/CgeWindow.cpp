#include "system/engine.h"
#include "system/application.h"
#include "window/window.h"
#include "system/utilities.h"
#include "system/script.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "io/Tracing.h"
#include "io/TraceManager.h"
#include "window/splashwindow.h"
#include "platform/userdirs.h"

#include "AdvDoc.h"
#include "Engine.h"
#include "Graphics.h"
#include "Sound.h"
#include "CmdReceiver.h"

using namespace adv;

TR_CHANNEL(ADV_CGE_Window);

// On the web, the browser runs the frames on its main thread, which must not
// block: no sleeping between frames and no debug ports (a server thread).
// The game is a data package that the page loads to data/game.dat (see
// test/CMakeLists.txt), and the traces go to the browser's console.
#ifdef __EMSCRIPTEN__
static const bool debugPorts = false;
#else
static bool debugPorts = false;
#endif

String filename;
std::string savegame;
AdvDocument* adoc = nullptr;
CommandReceiver receiver;
bool openConsole;
CGE::SplashWindow* splashWindow = nullptr;

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
  CGE::AppWindow* window = CGE::Engine::instance()->getWindow();
  window->changeSize(windowsize.x, windowsize.y);
  //the game and its input use the project resolution
  CGE::Engine::instance()->getVirtualScreen().setSize(windowsize.x, windowsize.y);

  Engine::instance()->setData(adoc);
  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->setColor(1.0f, 1.0f, 1.0f, 1.0f);
  gfx->setBlend(Graphics::BLEND_ALPHA);

  if (debugPorts)
    receiver.start();
  Engine::instance()->initGame(quit, set_mouse);
  delete splashWindow;
  splashWindow = nullptr;
  window->show(true);
}

void deinit(){
  Engine::instance()->exitGame();
  if (debugPorts)
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
  image.setData(nullptr);
}

void render(){
#ifndef __EMSCRIPTEN__
  double start = getTime();
#endif
  //the game draws into the window with y down, in its resolution
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->setBlend(Graphics::BLEND_ALPHA);
  gfx->setDepth(false, false);
  gfx->setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  gfx->beginFrame();

  receiver.processCommands();
  unsigned time = (unsigned)(CGE::Engine::instance()->getFrameInterval()*1000);
  Engine::instance()->render(time);

  SoundEngine::instance()->update(time);
  gfx->endFrame();

#ifndef __EMSCRIPTEN__
  double diff = getTime()-start;
  int sleeptime = 5 - (int)(diff*1000);
  if (sleeptime < 0)
    sleeptime = 0;

  CGE::Thread::sleep(sleeptime); //just a little sleep to avoid frame times < 5ms.
#endif

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

//! a typed character (a code point) in Windows-1252, the games' encoding
//! (what the ANSI window gave on Windows); 0 if it has none
unsigned char toWindows1252(unsigned int ch){
  //0x80 to 0x9F, which aren't Latin-1's
  static const unsigned short extra[32] = {
    0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
    0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178};
  if (ch < 0x80 || (ch >= 0xA0 && ch < 0x100))
    return (unsigned char)ch;
  for (int i = 0; i < 32; ++i){
    if (extra[i] != 0 && extra[i] == ch)
      return (unsigned char)(0x80 + i);
  }
  return 0;
}

void key_char(unsigned int ch){
  unsigned char chr = toWindows1252(ch);
  if (chr != 0)
    Engine::instance()->keyAscii((char)chr);
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
  void onChar(unsigned int ch) override {key_char(ch);}
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
#ifdef __EMSCRIPTEN__
  //the saves stay where the old web port had them, in /IDBFS/adventure/<game>
  CGE::UserDirs::setAppName("adventure");
#else
  debugPorts = CGE::Script::instance()->getBoolSetting("debugPorts");
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
#endif
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
