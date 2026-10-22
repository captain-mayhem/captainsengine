#include "engine.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
#include "../window/nativeAndroid.h"
#include "script.h"
#include "../renderer/drawcontext.h"
#include "../renderer/font.h"
#include "../renderer/forms.h"
#include "../math/matrixstack.h"
#include "../io/Tracing.h"
#include "../io/TraceManager.h"
#include "../platform/settings.h"
#include "../platform/platform_internal.h"

#include <algorithm>
#include <chrono>
#include <thread>

using std::string;
using std::list;

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <mmsystem.h>
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifdef ANDROID
#include <android_native_app_glue.h>
#endif
#ifdef LINUX
//XLookupString
#include <X11/Xutil.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
//after the X11 headers: imgui.h undefines X11's Status
#include "../ui/console.h"
#include "../ui/imgui_impl_cge_platform.h"
#include "../ui/imgui_impl_cge_render.h"

using namespace CGE;

TR_CHANNEL(CGE_Engine);

//! the application given to runApplication (application.cpp); it outlives
//! the engine instance, which Android recreates with its window
IApplication* internalApplication = nullptr;

Engine* Engine::eng = nullptr;

//! the most time the updates catch up with after a long frame
static const double MAX_UPDATE_LAG = 0.25;

Engine::Engine(){
  TR_USE(CGE_Engine);
  isUp_ = false;
  app_ = nullptr;
  paused_ = false;
  rendering_ = false;
  maxFramerate_ = 0;
  updateInterval_ = 1.0/60;
  updateLag_ = 0;
  frameInterval_ = 0;
  frameTime_ = 0;
  lastTime_ = 0;
  fpscounter_ = 0;
  fps_ = 0;
  imgui_ = false;
  mShutdownRequested = false;
#ifdef LINUX
  lastClick_ = 0;
#endif
#ifdef WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
  TR_INFO("Engine instance created");
}

Engine::~Engine(){
}

void Engine::init(){
  eng = new Engine();
}

void Engine::startup(int argc, char** argv){
  TR_USE(CGE_Engine);
  Script::init();
  Script::instance()->initEnv();
  PlatformInternal::applyEngineSettings(Script::instance()->getStringSetting("assetDir", false),
    Script::instance()->getStringSetting("appName", false));
  //the render backend
  string type = Script::instance()->getStringSetting("renderer");
  if (type.empty())
    type = "OpenGL";
  maxFramerate_ = Script::instance()->getNumberSetting("maxFramerate");
  int updateRate = Script::instance()->getNumberSetting("updateRate");
  if (updateRate <= 0)
    updateRate = 60;
  updateInterval_ = 1.0/updateRate;
  int virtualWidth = Script::instance()->getNumberSetting("virtualWidth");
  int virtualHeight = Script::instance()->getNumberSetting("virtualHeight");
  if (virtualWidth > 0 && virtualHeight > 0)
    virtual_.setSize(virtualWidth, virtualHeight);
  bool exists;
  graphics_ = Script::instance()->getBoolSetting("graphics", &exists);
  if (!exists)
    graphics_ = true;

  Gfx::DeviceDesc deviceDesc;
  deviceDesc.vsync = Script::instance()->getBoolSetting("vsync", &exists) || !exists;
  int bpp = Script::instance()->getNumberSetting("bitsPerPixel");
  if (bpp > 0)
    deviceDesc.colorBits = bpp;
#ifndef NDEBUG
  deviceDesc.debug = true;
#endif

  if (graphics_){
    device_ = Gfx::RenderDevice::create(type);
    if (!device_){
      EXIT2("The render backend %s is not available", type.c_str());
      return;
    }

#ifdef WIN32
    win_ = std::make_unique<WindowsWindow>(device_.get(), deviceDesc);
#endif
#if defined(LINUX)
    win_ = std::make_unique<X11Window>(device_.get(), deviceDesc);
#endif
#ifdef ANDROID
    win_ = std::make_unique<AndroidWindow>(device_.get(), deviceDesc);
#endif
  }
  else{
    device_ = Gfx::RenderDevice::create("Null");
    device_->init(Gfx::NativeWindow(), deviceDesc);
  }
  if (graphics_){
#ifdef WIN32
    //1 ms sleep resolution for the frame limiter
    timeBeginPeriod(1);
#endif
    win_->init("Captains Game Engine");
#ifdef LINUX
    XSynchronize(static_cast<X11Window*>(win_.get())->getDisplay(), true);
#endif
    if (!device_->init(win_->getNativeWindow(), deviceDesc)){
      EXIT2("The render device could not be initialised");
      win_->kill();
      return;
    }
    win_->setWidth(device_->getWidth());
    win_->setHeight(device_->getHeight());
  }
  draw_ = std::make_unique<Gfx::DrawContext>(*device_);
  Input::Keyboard::init();
  Input::Mouse::init(graphics_);
  
  if (graphics_){
    for (auto& fnt : fnt_){
      fnt = std::make_unique<CGE::Font>(*device_);
    }
    forms_ = std::make_unique<CGE::Forms>(*device_);
    startupImGui();
  }
  console_ = std::make_unique<CGE::Console>();
  isUp_ = true;
  app_ = internalApplication;
  if (app_)
    app_->init(argc, argv);
  frameTime_ = getTime();
  updateLag_ = 0;
}

void Engine::startupImGui(){
  TR_USE(CGE_Engine);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  //applications that want ImGui's window settings saved set a file name
  io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplCGE_Platform_Init(win_.get());
  //the context stays without a working renderer, so applications can still call ImGui
  if (!ImGui_ImplCGE_Render_Init(*device_))
    TR_WARN("ImGui can't draw on this render device");
  imgui_ = true;
}

void Engine::shutdownImGui(){
  if (!imgui_)
    return;
  ImGui_ImplCGE_Render_Shutdown();
  ImGui_ImplCGE_Platform_Shutdown();
  ImGui::DestroyContext();
  imgui_ = false;
}

void Engine::shutdown(){
  TR_USE(CGE_Engine);
  if (isUp_){
    if (app_)
      app_->shutdown();
    app_ = nullptr;
    Settings::saveUser();
    TR_INFO("engine shutting down");
    isUp_ = false;
    Input::Keyboard::release();
    Input::Mouse::release();
    console_.reset();
    //its textures are on the device
    shutdownImGui();
    for (auto& fnt : fnt_)
      fnt.reset();
    forms_.reset();
    draw_.reset();
    //the resources go before the context, the context before the window
    device_->shutdown();
    if (win_)
      win_->kill();
    win_.reset();
    device_.reset();
#ifdef WIN32
    if (graphics_)
      timeEndPeriod(1);
#endif
    Script::kill();
  }
  SAFE_DELETE(eng);
  TraceManager::deinit();
  //exit(0);
}

void Engine::runFrame(){
  processEvents();
  frame(true);
}

bool Engine::pumpFrame(){
  TR_USE(CGE_Engine);
  if (rendering_){
    TR_ERROR("pumpFrame called while rendering");
    return !mShutdownRequested;
  }
  processEvents();
  frame(false);
#if defined(__EMSCRIPTEN__) && !defined(ENGINE_SINGLE_THREADED)
  //hand control to the browser until the next frame (needs ASYNCIFY)
  emscripten_sleep(0);
#endif
  return !mShutdownRequested;
}

void Engine::frame(bool update){
  if (mShutdownRequested || !isUp_)
    return;
  double currentTime = getTime();
  frameInterval_ = currentTime-frameTime_;
  frameTime_ = currentTime;

  fpscounter_++;
  //one second passed by
  if (currentTime - lastTime_ > 1.0){
    lastTime_ = currentTime;
    fps_ = fpscounter_;
    fpscounter_ = 0;
  }

  console_->update();

  if (paused_){
    //nothing to do in the background
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return;
  }

  if (update && app_){
    updateLag_ = (std::min)(updateLag_+frameInterval_, MAX_UPDATE_LAG);
    while (updateLag_ >= updateInterval_ && !mShutdownRequested){
      app_->update(updateInterval_);
      updateLag_ -= updateInterval_;
    }
  }

  if (!mShutdownRequested)
    renderFrame();
  limitFrame();
}

void Engine::renderFrame(){
  if (!graphics_)
    return;
  if (!device_->beginFrame())
    return;
  rendering_ = true;
  //the application builds its ImGui windows in render
  if (imgui_){
    ImGui_ImplCGE_Render_NewFrame();
    ImGui_ImplCGE_Platform_NewFrame();
    ImGui::NewFrame();
  }
  if (app_)
    app_->render();

  renderFonts();

  //ImGui on top of everything, the console last
  if (imgui_){
    console_->render();
    renderImGui();
  }

  device_->endFrame();
  rendering_ = false;
}

void Engine::renderFonts(){
  bool texts = false;
  for (auto& fnt : fnt_)
    texts = texts || !fnt->isEmpty();
  if (!texts)
    return;
  Gfx::PassDesc pass;
  pass.colorLoad = Gfx::LoadAction::Load;
  pass.depthLoad = Gfx::LoadAction::Load;
  if (!device_->beginPass(pass))
    return;
  //in the virtual screen with y up, added onto the frame
  draw_->setProjection(getVirtualScreen().ortho(*device_));
  draw_->setView(Matrix(Matrix::Identity));
  draw_->model().load(Matrix(Matrix::Identity));
  draw_->setBlend(Gfx::BlendMode::Additive);
  draw_->setDepth(false, false);
  draw_->setCull(Gfx::CullMode::Off);
  for (auto& fnt : fnt_)
    fnt->render(*draw_, frameInterval_);
  draw_->setBlend(Gfx::BlendMode::Opaque);
  device_->endPass();
}

void Engine::renderImGui(){
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  //the texture requests are handled also when nothing is drawn
  bool draw = drawData->TotalVtxCount > 0;
  if (draw){
    Gfx::PassDesc pass;
    pass.colorLoad = Gfx::LoadAction::Load;
    pass.depthLoad = Gfx::LoadAction::Load;
    draw = device_->beginPass(pass);
  }
  ImGui_ImplCGE_Render_RenderDrawData(drawData);
  if (draw)
    device_->endPass();
}

void Engine::limitFrame(){
  if (maxFramerate_ <= 0)
    return;
  //sleep for most of the remaining time and wait out the rest, because
  //sleeping is only accurate to about a millisecond
  double frameEnd = frameTime_ + 1.0/maxFramerate_;
  double remaining;
  while ((remaining = frameEnd-getTime()) > 0){
    if (remaining > 0.002)
      std::this_thread::sleep_for(std::chrono::duration<double>(remaining-0.0015));
    else
      std::this_thread::yield();
  }
}

void Engine::resize(int width, int height){
  if (!device_ || !win_)
    return;
  if (height == 0)
    height = 1;
  device_->resize(width, height);
  win_->setWidth(width);
  win_->setHeight(height);
  if (app_ && win_)
    app_->onResize(win_->getWidth(), win_->getHeight());
}

VirtualScreen& Engine::getVirtualScreen(){
  if (win_)
    virtual_.setWindowSize(win_->getWidth(), win_->getHeight());
  return virtual_;
}

void Engine::pause(){
  if (paused_)
    return;
  paused_ = true;
  if (app_)
    app_->onPause();
  //the system may end the application in the background (Android)
  Settings::saveUser();
}

void Engine::resume(){
  if (!paused_)
    return;
  paused_ = false;
  //don't catch up with the time spent in the background
  frameTime_ = getTime();
  updateLag_ = 0;
  if (app_)
    app_->onResume();
}

#ifdef WIN32
void Engine::processEvents(){
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (msg.message == WM_QUIT)
      break;
  }
  if (mShutdownRequested || !win_)
    return;
  //Do not use WM_MouseMove-Event, because it causes the cursor to freeze
  POINT p;
  GetCursorPos(&p);
  ScreenToClient((HWND)static_cast<CGE::WindowsWindow*>(win_.get())->getHandle(), &p);
  ImGui_ImplCGE_Platform_AddMousePos((float)p.x, (float)p.y);
  Input::Mouse::instance()->move(p.x, p.y, 0);
}
#endif

#ifdef LINUX
namespace{

//! passes the modifier state of an X11 event on to ImGui
void imguiModifiers(unsigned state){
  ImGui_ImplCGE_Platform_AddModifiers((state & ControlMask) != 0, (state & ShiftMask) != 0,
    (state & Mod1Mask) != 0, (state & Mod4Mask) != 0);
}

//! passes an X11 button event on to ImGui: buttons 1-3, or wheel notches (4-7)
void imguiButton(const XButtonEvent& event, bool down){
  imguiModifiers(event.state);
  ImGui_ImplCGE_Platform_AddMousePos((float)event.x, (float)event.y);
  switch (event.button){
    case Button1: ImGui_ImplCGE_Platform_AddMouseButton(ImGuiMouseButton_Left, down); break;
    case Button2: ImGui_ImplCGE_Platform_AddMouseButton(ImGuiMouseButton_Middle, down); break;
    case Button3: ImGui_ImplCGE_Platform_AddMouseButton(ImGuiMouseButton_Right, down); break;
    //a wheel notch is a press and a release
    case Button4: if (down) ImGui_ImplCGE_Platform_AddMouseWheel(0, 1); break;
    case Button5: if (down) ImGui_ImplCGE_Platform_AddMouseWheel(0, -1); break;
    case 6: if (down) ImGui_ImplCGE_Platform_AddMouseWheel(1, 0); break;
    case 7: if (down) ImGui_ImplCGE_Platform_AddMouseWheel(-1, 0); break;
    default: break;
  }
}

}

void Engine::processEvents(){
  if (!win_)
    return;
  CGE::X11Window* win = static_cast<CGE::X11Window*>(win_.get());
  Display* disp = win->getDisplay();
  XEvent event;
  int x = XPending(disp);
  while (x > 0){
    x--;
    XNextEvent(disp, &event);
    switch (event.type){
    case ConfigureNotify:
      //compared with the device: changeSize sets the window's size before the event arrives
      if (event.xconfigure.width != device_->getWidth() ||
        event.xconfigure.height != device_->getHeight()){
        resize(event.xconfigure.width, event.xconfigure.height);
      }
      break;
    case MapNotify:
      resume();
      break;
    case UnmapNotify:
      pause();
      break;
    case KeyPress:
    {
      imguiModifiers(event.xkey.state);
      ImGui_ImplCGE_Platform_AddKey((int)XLookupKeysym(&event.xkey, 0), true);
      char ascii;
      KeySym key;
      int num = XLookupString(&event.xkey, &ascii, 1, &key, nullptr);
      Input::Keyboard::instance()->keyDown(key);
      if (num > 0)
        Input::Keyboard::instance()->ascii(ascii);
      break;
    }
    case KeyRelease:
      imguiModifiers(event.xkey.state);
      ImGui_ImplCGE_Platform_AddKey((int)XLookupKeysym(&event.xkey, 0), false);
      Input::Keyboard::instance()->keyUp(XLookupKeysym(&event.xkey, 0));
      break;
    case ButtonPress:
      imguiButton(event.xbutton, true);
      if (event.xbutton.button > Button3){
        Input::Mouse::instance()->wheel(event.xbutton.x, event.xbutton.y, event.xbutton.button == Button4 ? 1 : -1);
      }
      else{
        double newClick = getTime();
        if (newClick - lastClick_ < 0.5){
          Input::Mouse::instance()->doubleClick(event.xbutton.x, event.xbutton.y, event.xbutton.button);
          lastClick_ = 0;
        }
        else{
          lastClick_ = getTime();
          Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, event.xbutton.button);
        }
      }
      break;
    case ButtonRelease:
      imguiButton(event.xbutton, false);
      if (event.xbutton.button <= Button3 && lastClick_ != 0)
        Input::Mouse::instance()->buttonUp(event.xbutton.x, event.xbutton.y, event.xbutton.button);
      break;
    case MotionNotify:
      ImGui_ImplCGE_Platform_AddMousePos((float)event.xmotion.x, (float)event.xmotion.y);
      Input::Mouse::instance()->move(event.xmotion.x, event.xmotion.y, event.xmotion.state);
      break;
    case LeaveNotify:
      ImGui_ImplCGE_Platform_AddMouseLeave();
      break;
    case FocusIn:
      ImGui_ImplCGE_Platform_AddFocus(true);
      break;
    case FocusOut:
      ImGui_ImplCGE_Platform_AddFocus(false);
      break;
    case ClientMessage:
      if (*XGetAtomName(disp, event.xclient.message_type) == *"WM_PROTOCOLS"){
        requestShutdown();
      }
      break;
    default:
      break;
    }
  }
}
#endif

#ifdef ANDROID
void Engine::processEvents(){
  //the loop in runApplication polls the looper, which calls handleCmd and
  //handleInput (redone with the Android port in Phase 1c)
}
#endif

#ifdef __EMSCRIPTEN__
void Engine::processEvents(){
  //the browser delivers events between frames (web backend in Phase 1c)
}
#endif

double getTime(){
  using namespace std::chrono;
  return duration<double>(steady_clock::now().time_since_epoch()).count();
}

void Engine::requestShutdown(){
  mShutdownRequested = true;
}

void Engine::messageBox(const std::string& message, const std::string& title){
  TR_USE(CGE_Engine);
  if (win_ == nullptr)
    TR_WARN("MsgBox failed (no window active): %s: %s", title.c_str(), message.c_str());
  else
    win_->messageBox(title, message);
}

#ifdef ANDROID
#include <android/sensor.h>
#include <android_native_app_glue.h>

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
};


/**
 * Process the next input event.
 */
int32_t Engine::handleInput(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    	engine->animating = 1;
    	int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
    switch(action){
    	case AMOTION_EVENT_ACTION_DOWN:
    	 Input::Mouse::instance()->buttonDown(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0), MB_LEFT);
    	break;
    	case AMOTION_EVENT_ACTION_UP:
    	 Input::Mouse::instance()->buttonUp(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0), MB_LEFT);
    	break;
    	case AMOTION_EVENT_ACTION_MOVE:
    	 Input::Mouse::instance()->move(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0), MB_LEFT);
    	break;
    }
    return 1;
  }
  return 0;
}

/**
 * Process the next main command.
 */
void Engine::handleCmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            //engine->app->savedState = malloc(sizeof(struct saved_state));
            //*((struct saved_state*)engine->app->savedState) = engine->state;
            //engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
        	if (!Engine::instance())
        		Engine::init();
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                static char title[] = "cge";
                char* argv[] = {title, nullptr};
                CGE::Engine::instance()->startup(1, argv);
                CGE::Engine::instance()->runFrame();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            CGE::Engine::instance()->shutdown();
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            if (CGE::Engine::instance())
                CGE::Engine::instance()->resume();
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            if (CGE::Engine::instance())
                CGE::Engine::instance()->pause();
            //engine_draw_frame
            break;
    }
}

#endif

IApplication::~IApplication(){
}

void IApplication::onKeyDown(int key){
  if (key == KEY_ESCAPE)
    EXIT();
}

int CGE::runApplication(int argc, char** argv, IApplication& app, void* data){
  internalApplication = &app;
  PlatformInternal::init(argc, argv, data);
  CGE::Engine::init();

#if defined(__EMSCRIPTEN__)
  //the user folders live in IndexedDB: mount them and load their contents
  //before the engine starts (see UserDirs)
  EM_ASM(
    Module.cgeFsReady = 0;
    FS.mkdir('/IDBFS');
    FS.mount(IDBFS, {}, '/IDBFS');
    FS.syncfs(true, function(err){
      if (err)
        console.error("CGE: loading from IndexedDB failed", err);
      Module.cgeFsReady = 1;
    });
  );
  static int startupArgc = argc;
  static char** startupArgv = argv;
  //the browser calls runFrame once per frame; main returns right away
  emscripten_set_main_loop([](){
    static bool started = false;
    if (!started){
      if (!EM_ASM_INT(return Module.cgeFsReady;))
        return;
      started = true;
      CGE::Engine::instance()->startup(startupArgc, startupArgv);
    }
    CGE::Engine::instance()->runFrame();
    if (CGE::Engine::instance()->isShutdownRequested()){
      emscripten_cancel_main_loop();
      CGE::Engine::instance()->shutdown();
      internalApplication = nullptr;
    }
  }, 0, false);
  return 0;
#elif !defined(ANDROID)
  CGE::Engine::instance()->startup(argc, argv);
  while (!CGE::Engine::instance()->isShutdownRequested()){
    CGE::Engine::instance()->runFrame();
  }
  CGE::Engine::instance()->shutdown();
  internalApplication = nullptr;
  return 0;
#else
  struct android_app* state = (android_app*)data;
  CGE::Engine::instance()->m_app = state;
  struct engine engine;

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = Engine::handleCmd;
  state->onInputEvent = Engine::handleInput;
  engine.app = state;

  // Prepare to monitor accelerometer
  engine.sensorManager = ASensorManager_getInstance();
  engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
    ASENSOR_TYPE_ACCELEROMETER);
  engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
    state->looper, LOOPER_ID_USER, nullptr, nullptr);

  TR_USE(CGE_Engine);
  while (1){
    int ident;
    int events;
    struct android_poll_source* source;
    while ((ident = ALooper_pollOnce(engine.animating ? 0 : -1, nullptr, &events, (void**)&source)) >= 0){
      // Process this event.
      if (source != nullptr){
        source->process(state, source);
      }

      // If a sensor has data, process it now.
      if (ident == LOOPER_ID_USER){
        if (engine.accelerometerSensor != nullptr){
          ASensorEvent event;
          while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0){
            TR_DEBUG("accelerometer: x=%f y=%f z=%f",
              event.acceleration.x, event.acceleration.y,
              event.acceleration.z);
          }
        }
      }
      if (state->destroyRequested){
        if (CGE::Engine::instance())
          CGE::Engine::instance()->shutdown();
        internalApplication = nullptr;
        return 0;
      }

      if (engine.animating && CGE::Engine::instance()){
        // Drawing is throttled to the screen update rate, so there
        // is no need to do timing here.
        CGE::Engine::instance()->runFrame();
      }
    }
  }
  return 0;
#endif
}

#ifdef WIN32
int CGE::runApplication(const char* cmdline, IApplication& app){
  //_CrtSetBreakAlloc(729);
  std::vector<std::string> args;
  std::vector<const char*> argv;
  args.push_back("Captains Game Engine");
  int len = (int)strlen(cmdline);
  std::string curr;
  bool acceptWs = false;
  for (int i = 0; i < len; ++i){
    if (cmdline[i] == '"'){
      acceptWs = !acceptWs;
      continue;
    }
    if (!acceptWs && cmdline[i] == ' ' && !curr.empty()){
      args.push_back(curr);
      curr.clear();
    }
    else
      curr.append(1, cmdline[i]);
  }
  if (!curr.empty()){
    args.push_back(curr);
    curr.clear();
  }
  argv.resize(args.size());
  for (unsigned i = 0; i < args.size(); ++i){
    argv[i] = args[i].c_str();
  }
  return runApplication((int)argv.size(), (char**)&argv[0], app);
}
#endif
