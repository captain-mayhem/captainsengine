#include "system/engine.h"
#include "system/application.h"
#include "system/file.h"
#include "window/window.h"
#include "system/utilities.h"
#include "system/script.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/touch.h"
#include "io/Tracing.h"
#include "io/TraceManager.h"
#include "window/splashwindow.h"
#include "platform/userdirs.h"

#include "AdvDoc.h"
#include "Engine.h"
#include "Graphics.h"
#include "Sound.h"
#include "CmdReceiver.h"

#include <cstdio>
#include <cstdlib>

using namespace adv;

TR_CHANNEL(ADV_CGE_Window);

// On the web, the browser runs the frames on its main thread, which must not
// block: no sleeping between frames and no debug ports (a server thread).
// The game is a data package that the page loads to data/game.dat (see
// test/CMakeLists.txt), and the traces go to the browser's console.
// On Android, the launcher activity (Adventure/android) passes the game's
// game.dat in the app's storage as the first argument, there are no debug
// ports either, and the traces go to logcat.
#if defined(__EMSCRIPTEN__) || defined(ANDROID)
static const bool debugPorts = false;
#else
static bool debugPorts = false;
#endif

String filename;
std::string savegame;
//! a saved state to load after the next frame, and whether to delete it then
std::string loadState;
bool deleteLoadedState = false;
//! the game runs (initGame done, not ended)
bool gameStarted = false;
//! the game ended itself (quit), so it doesn't resume where it was
bool gameQuit = false;
//! the state saved while the render device restarts
std::string restartState;
AdvDocument* adoc = nullptr;
CommandReceiver receiver;
bool openConsole;
CGE::SplashWindow* splashWindow = nullptr;

Vec2i windowsize(640,480);

void quit();
void set_mouse(int x, int y);
void touch_update();
void splash_screen(unsigned width, unsigned height, unsigned numChannels, void* data);

void fileChanged(std::string const& file){
  TR_USE(ADV_CGE_Window);
  TR_INFO("File %s written", file.c_str());
}

//! a file for the game's state in its save folder
std::string statePath(const char* name){
  return Engine::instance()->getSettings()->savedir + "/" + name;
}

//! the state Android's autosave writes when the app goes into the
//! background, since the system may end the process there; the next start
//! continues from it, unless the game ended itself. It is only written when
//! a save holds the whole state (Engine::isSaveable: not during a cutscene or
//! a room change); otherwise the previous one stays.
std::string resumePath(){
  return statePath("resume.sav");
}

//! starts the game in a new adventure engine (Engine::init has been called)
//! on the loaded document
void startGame(){
  Engine::instance()->setData(adoc);

  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->setColor(1.0f, 1.0f, 1.0f, 1.0f);
  gfx->setBlend(Graphics::BLEND_ALPHA);

  Engine::instance()->initGame(quit, set_mouse);
  gameStarted = true;
}

//! saves the running game's state to path
void saveState(const std::string& path){
  if (gameStarted)
    Engine::instance()->getSaver()->save(path);
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

  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  if (debugPorts)
    receiver.start();
  startGame();
  delete splashWindow;
  splashWindow = nullptr;
  window->show(true);
}

void deinit(){
  if (Engine::instance())
    Engine::instance()->exitGame();
  if (debugPorts)
    receiver.stop();
#ifdef ANDROID
  if (gameStarted && gameQuit)
    std::remove(resumePath().c_str());
#endif
  gameStarted = false;

  SoundEngine::deinit();
  Engine::deinit();
  delete adoc;
}

void quit(){
  gameQuit = true;
  CGE::Engine::instance()->requestShutdown();
}

//! the render device lost its resources: the game is saved and ended with
//! all its graphics (textures, render targets, shaders of the rooms, fonts
//! and effects). During a cutscene or a room change a save would miss parts
//! of the state, so the game continues from the last autosave then, if there
//! is one.
void deviceLost(){
  if (!gameStarted)
    return;
  restartState = statePath("restart.sav");
#ifdef ANDROID
  if (!Engine::instance()->isSaveable() && CGE::Filesystem::exists(resumePath())){
    TR_USE(ADV_CGE_Window);
    TR_WARN("The render device restarts during a cutscene or a room change: continuing from the autosave");
    restartState = resumePath();
  }
  else
#endif
  saveState(restartState);
  Engine::instance()->exitGame();
  Engine::deinit();
  gameStarted = false;
}

//! the render device is back: the game starts again and loads the state
//! saved in deviceLost after the next frame, as a saved game is loaded
void deviceRestored(){
  if (restartState.empty())
    return;
  Engine::init();
  startGame();
  loadState = restartState;
  //the autosave stays, for the next start
  deleteLoadedState = restartState != resumePath();
  restartState.clear();
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

  touch_update();

  if (!savegame.empty()){
    loadState = Engine::instance()->getSaver()->saveSlotToPath(atoi(savegame.c_str()));
    deleteLoadedState = false;
    savegame.clear();
  }
  if (!loadState.empty()){
    Engine::instance()->getSaver()->load(loadState, false);
    if (deleteLoadedState)
      std::remove(loadState.c_str());
    loadState.clear();
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

//! Touch (the engine's mouse emulation is off): the cursor follows the
//! finger, so the hover texts show while it moves; lifting it clicks there.
//! A long press without moving does what the right button does (e.g. the
//! next command, "Look at"), and two taps in a row are a double click. Only
//! the first finger on the screen counts.
const double longPressTime = 0.5;
const double doubleTapTime = 0.5;
int touchId = -1;
Vec2i touchStart;
double touchStartTime = 0;
bool touchMoved = false;
bool touchLongPressed = false;
double lastTapTime = -1;
Vec2i lastTapPos;

//! how far a finger may move and still press or tap at its place
bool touchNear(int x, int y, const Vec2i& pos){
  int slop = windowsize.x/40 > 4 ? windowsize.x/40 : 4;
  return abs(x - pos.x) <= slop && abs(y - pos.y) <= slop;
}

void touch_down(int id, int x, int y){
  if (Input::Touch::instance()->getPrimary() != id)
    return;
  touchId = id;
  touchStart = Vec2i(x, y);
  touchStartTime = getTime();
  touchMoved = false;
  touchLongPressed = false;
  mouse_move(x, y, 0);
}

void touch_move(int id, int x, int y){
  if (id != touchId)
    return;
  mouse_move(x, y, 0);
  if (!touchNear(x, y, touchStart))
    touchMoved = true;
}

void touch_up(int id, int x, int y){
  if (id != touchId)
    return;
  touchId = -1;
  mouse_move(x, y, 0);
  if (touchLongPressed)
    return;
  double now = getTime();
  if (lastTapTime >= 0 && now - lastTapTime < doubleTapTime && touchNear(x, y, lastTapPos)){
    double_click(x, y, MB_LEFT);
    lastTapTime = -1;
  }
  else{
    mouse_click(x, y, MB_LEFT);
    lastTapTime = now;
    lastTapPos = Vec2i(x, y);
  }
  mouse_release(x, y, MB_LEFT);
}

void touch_cancel(int id){
  if (id == touchId)
    touchId = -1;
}

//! the long press, once per frame
void touch_update(){
  if (touchId < 0 || touchMoved || touchLongPressed || getTime() - touchStartTime < longPressTime)
    return;
  touchLongPressed = true;
  lastTapTime = -1;
  mouse_click(touchStart.x, touchStart.y, MB_RIGHT);
}

void key_press(int key){
  //Ctrl+R restarts the render device, as after a lost context (Android), to test that path
  if (key == KEY_R && Input::Keyboard::instance()->isPressed(KEY_CTRL)){
    CGE::Engine::instance()->restartRenderDevice();
    return;
  }
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
  void onTouchDown(int id, int x, int y) override {touch_down(id, x, y);}
  void onTouchMove(int id, int x, int y) override {touch_move(id, x, y);}
  void onTouchUp(int id, int x, int y) override {touch_up(id, x, y);}
  void onTouchCancel(int id) override {touch_cancel(id);}
  void onPause() override;
  void onDeviceLost() override {deviceLost();}
  void onDeviceRestored() override {deviceRestored();}
};

void AdventureApp::onPause(){
#ifdef ANDROID
  //the system may end the process in the background, without a shutdown
  if (!gameQuit && gameStarted && Engine::instance()->isSaveable())
    saveState(resumePath());
#endif
}

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
#elif !defined(ANDROID)
  debugPorts = CGE::Script::instance()->getBoolSetting("debugPorts");
#endif
#if !defined(__EMSCRIPTEN__) && !defined(ANDROID)
  //a log file; the web and Android trace to the browser's console and logcat
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
  //touches are taps and long presses (touch_down), not a mouse button
  if (Input::Touch::instance())
    Input::Touch::instance()->setMouseEmulation(false);
  ::init();
#ifdef ANDROID
  //continues where the game was when the system ended the process
  if (gameStarted && savegame.empty() && CGE::Filesystem::exists(resumePath())){
    loadState = resumePath();
    deleteLoadedState = true;
  }
#endif
}

CGE_MAIN(AdventureApp)
