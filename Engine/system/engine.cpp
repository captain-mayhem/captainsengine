#include "engine.h"
#include "../renderer/GL2/GL2Renderer.h"
#include "../renderer/OpenGL/OGLrenderer.h"
#include "../renderer/DirectX/DXrenderer.h"
#include "../renderer/dummyrenderer.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
#include "../window/nativeAndroid.h"
#include "script.h"
#include "../gui/console.h"
#include "../renderer/forms.h"
#include "../io/Tracing.h"
#include "../io/TraceManager.h"

#include <algorithm>
#include <chrono>
#include <thread>

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <mmsystem.h>
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifdef ANDROID
#include <android_native_app_glue.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace CGE;

TR_CHANNEL(CGE_Engine);

//! the application given to runApplication (application.cpp); it outlives
//! the engine instance, which Android recreates with its window
IApplication* internalApplication = nullptr;

Engine* Engine::eng = NULL;

//! the most time the updates catch up with after a long frame
static const double MAX_UPDATE_LAG = 0.25;

Engine::Engine(){
  TR_USE(CGE_Engine);
  win_ = NULL;
  rend_ = NULL;
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
  input_ = NULL;
  console_ = NULL;
  clear_ = false;
  //guitex_ = Mutex();
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

void Engine::init(){
  eng = new Engine();
}

void Engine::startup(int argc, char** argv){
  TR_USE(CGE_Engine);
  Script::init();
  Script::instance()->initEnv();
  string type = Script::instance()->getStringSetting("renderer");
  if (type.empty())
    type = "GL2";
  maxFramerate_ = Script::instance()->getNumberSetting("maxFramerate");
  int updateRate = Script::instance()->getNumberSetting("updateRate");
  if (updateRate <= 0)
    updateRate = 60;
  updateInterval_ = 1.0/updateRate;
  bool exists;
  graphics_ = Script::instance()->getBoolSetting("graphics", &exists);
  if (!exists)
    graphics_ = true;

  if (graphics_){
    if (!startupRenderer(type)){
      EXIT2("No valid renderer specified in engine.ini");
    }

#ifdef WIN32
    win_ = new WindowsWindow(rend_);
#endif
#if defined(LINUX)
    win_ = new X11Window(rend_);
#endif
#ifdef ANDROID
  win_ = new AndroidWindow(rend_);
#endif
  }
  else{
    rend_ = new CGE::DummyRenderer();
  }
  if (graphics_){
#ifdef WIN32
    //1 ms sleep resolution for the frame limiter
    timeBeginPeriod(1);
#endif
    win_->init("Captains Game Engine");
#ifdef LINUX
    XSynchronize(static_cast<X11Window*>(win_)->getDisplay(), true);
#endif
  }
  Input::Keyboard::init();
  Input::Mouse::init(graphics_);
  
  fnt_ = new CGE::Font*[3];
  if (graphics_){
    for (int i = 0; i < 3; i++){
      fnt_[i] = new CGE::Font();
    }
    fnt_[0]->buildFont(NULL);
    fnt_[1]->buildFont(fnt_[0]->getVB());
    fnt_[2]->buildFont(fnt_[0]->getVB());
    forms_ = new CGE::Forms();
  }
  else{
    for (int i = 0; i < 3; i++){
      fnt_[i] = NULL;
    }
  }
  console_ = new ::Gui::Console();
  isUp_ = true;
  rend_->initRendering();
  app_ = internalApplication;
  if (app_)
    app_->init(argc, argv);
  frameTime_ = getTime();
  updateLag_ = 0;
}

bool Engine::startupRenderer(const std::string& type){
  TR_USE(CGE_Engine);
  if (type == "OpenGL"){
#ifdef OPENGL
    rend_ = new CGE::OGLRenderer();
#else
    TR_ERROR("OpenGL support is not compiled in");
#endif
  }
  else if (type == "DirectX"){
#ifdef DIRECTX
    rend_ = new CGE::DXRenderer();
#else
    TR_ERROR("DirectX support is not compiled in\n");
#endif
  }
  else if (type == "GLES"){
#ifdef GLES
    rend_ = new CGE::GLESRenderer();
#else
    TR_ERROR("OpenGL ES support is not compiled in\n");
#endif
  }
  else if (type == "GL2"){
#ifdef OPENGL2
    rend_ = new CGE::GL2Renderer();
#else
    TR_ERROR("OpenGL (ES) 2 support is not compiled in\n");
#endif
  }
  return rend_ != NULL;
}

void Engine::shutdown(){
  TR_USE(CGE_Engine);
  if (isUp_){
    if (app_)
      app_->shutdown();
    app_ = nullptr;
    rend_->deinitRendering();
    TR_INFO("engine shutting down");
    isUp_ = false;
    Input::Keyboard::release();
    Input::Mouse::release();
    clearListeners();
    SAFE_DELETE(console_);
    fnt_[0]->killFont();
    for (int i = 0; i < 3; i++){
      if (i != 0)
        fnt_[i]->resetVB();
      SAFE_DELETE(fnt_[i]);
    }
    SAFE_DELETE_ARRAY(fnt_);
    SAFE_DELETE(forms_);
    if (win_)
      win_->kill();
    SAFE_DELETE(win_);
    SAFE_DELETE(rend_);
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
  updateGuiElements();

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

void Engine::updateGuiElements(){
  //something with the GUI elements changed, so update here
  for (unsigned i = 0; i < remBut_.size(); i++){
    removeGuiListener(remBut_[i]);
  }
  remBut_.clear();
  if (newBut_.size() != 0 || clear_){
    clear_ = false;
    clearListeners();
    guiElems_ = newBut_;
    newBut_.clear();
  }
}

void Engine::renderFrame(){
  if (!graphics_)
    return;
  rendering_ = true;
  //render scene
  rend_->renderScene();
  if (app_)
    app_->render();

  rend_->resetModelView();
  rend_->ortho(1024, 768);
  rend_->translate(-512, -384, 0);
  rend_->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE);
  rend_->enableBlend(true);
  rend_->enableDepthTest(false);
  rend_->enableTexturing(true);

  //render GUI-elements
  guitex_.lock();

  fnt_[0]->render();

  list< ::Gui::GuiElement*>::iterator iter2;
  for (iter2 = guiElems_.begin(); iter2 != guiElems_.end(); iter2++){
    (*iter2)->render();
  }

  fnt_[1]->render();

  CGE::Engine::instance()->getConsole()->render();

  fnt_[2]->render();

  guitex_.unlock();

  rend_->enableBlend(false);
  rend_->enableDepthTest(true);

  rend_->swapBuffers();
  rendering_ = false;
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
  if (!rend_)
    return;
  rend_->resizeScene(width, height);
  if (app_ && win_)
    app_->onResize(win_->getWidth(), win_->getHeight());
}

void Engine::pause(){
  if (paused_)
    return;
  paused_ = true;
  if (app_)
    app_->onPause();
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
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
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
  ScreenToClient((HWND)static_cast<CGE::WindowsWindow*>(win_)->getHandle(), &p);
  Input::Mouse::instance()->move(p.x, p.y, 0);
}
#endif

#ifdef LINUX
void Engine::processEvents(){
  if (!win_)
    return;
  CGE::X11Window* win = static_cast<CGE::X11Window*>(win_);
  Display* disp = win->getDisplay();
  XEvent event;
  int x = XPending(disp);
  while (x > 0){
    x--;
    XNextEvent(disp, &event);
    switch (event.type){
    case ConfigureNotify:
      if (event.xconfigure.width != win->getWidth() ||
        event.xconfigure.height != win->getHeight()){
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
      char ascii;
      KeySym key;
      int num = XLookupString(&event.xkey, &ascii, 1, &key, NULL);
      Input::Keyboard::instance()->keyDown(key);
      if (num > 0)
        Input::Keyboard::instance()->ascii(ascii);
      break;
    }
    case KeyRelease:
      Input::Keyboard::instance()->keyUp(XLookupKeysym(&event.xkey, 0));
      break;
    case ButtonPress:
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
      if (event.xbutton.button <= Button3 && lastClick_ != 0)
        Input::Mouse::instance()->buttonUp(event.xbutton.x, event.xbutton.y, event.xbutton.button);
      break;
    case MotionNotify:
      Input::Mouse::instance()->move(event.xmotion.x, event.xmotion.y, event.xmotion.state);
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

// set the current input field that should be active
void Engine::setActiveInput(::Gui::InputField* field){
  if (field == NULL){
    input_ = NULL;
    return;
  }
  if (input_ != NULL){
    input_->removeChar();
  }
  input_ = field;
  input_->addChar('_');
}

// clears the input and button listeners
void Engine::clearListeners(bool immediate){
  if (!immediate){
    clear_ = true;
    return;
  }
  guitex_.lock();
  list< ::Gui::GuiElement* >::iterator iter2;
  for (iter2 = guiElems_.begin(); iter2 != guiElems_.end(); iter2++){
    delete (*iter2);
  }
  guiElems_.clear();
  fnt_[0]->clear();
  fnt_[1]->clear();
  fnt_[2]->clear();
  guitex_.unlock();
}


// remove the input listener at position idx
/*
void Engine::removeInputListener(int idx){
  guitex_.lock();
  list< ::Gui::InputField*>::iterator iter = listeners_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  listeners_.erase(iter);
  guitex_.unlock();
}*/

// remove the button listener at position idx
void Engine::removeGuiListener(int idx, bool immediate){
  if (!immediate){
    remBut_.push_back(idx);
    return;
  }
  guitex_.lock();
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  delete *iter;
  guiElems_.erase(iter);
  guitex_.unlock();
}


// remove the button listener with a certain name
void Engine::removeGuiListener(const string& name){
  guitex_.lock();
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (unsigned i = 0; i < guiElems_.size(); i++){
    if((*iter)->getName() == name){
      delete *iter;
      guiElems_.erase(iter);
      break;
    }
    iter++;
  }
  guitex_.unlock();
}


// remove all button listeners beginning at idx
void Engine::removeGuiListeners(int idx){
  guitex_.lock();
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  while (iter != guiElems_.end()){
    delete *iter;
    iter = guiElems_.erase(iter);
  }
  guitex_.unlock();
}

// get a button by name
::Gui::GuiElement* Engine::getGuiListener(const string& name){
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (unsigned i = 0; i < guiElems_.size(); i++){
    if ((*iter)->getName() == name){
      return *iter;
    }
    iter++;
  }
  return NULL;
}

double getTime(){
  using namespace std::chrono;
  return duration<double>(steady_clock::now().time_since_epoch()).count();
}

void Engine::requestShutdown(){
  mShutdownRequested = true;
}

void Engine::messageBox(const std::string& message, const std::string& title){
  TR_USE(CGE_Engine);
  if (win_ == NULL)
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
            if (engine->app->window != NULL) {
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
            if (engine->accelerometerSensor != NULL) {
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
            if (engine->accelerometerSensor != NULL) {
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
  CGE::Engine::init();

#if defined(__EMSCRIPTEN__)
  //the browser calls runFrame once per frame; main returns right away
  CGE::Engine::instance()->startup(argc, argv);
  emscripten_set_main_loop([](){
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
    state->looper, LOOPER_ID_USER, NULL, NULL);

  TR_USE(CGE_Engine);
  while (1){
    int ident;
    int events;
    struct android_poll_source* source;
    while ((ident = ALooper_pollOnce(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0){
      // Process this event.
      if (source != NULL){
        source->process(state, source);
      }

      // If a sensor has data, process it now.
      if (ident == LOOPER_ID_USER){
        if (engine.accelerometerSensor != NULL){
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
