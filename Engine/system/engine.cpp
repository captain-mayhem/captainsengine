#include "engine.h"
#include "../renderer/GL2/GL2Renderer.h"
#include "../renderer/OpenGL/OGLrenderer.h"
#include "../renderer/DirectX/DXrenderer.h"
#include "../renderer/dummyrenderer.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
#include "../window/nativeQNX.h"
#include "../window/nativeAndroid.h"
#include "script.h"
#include "../gui/console.h"
#include "../renderer/forms.h"
#include "../physics/Simulator.h"
#include "../io/Tracing.h"
#include "../io/TraceManager.h"

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <mmsystem.h>
#include <stdlib.h>
#include <crtdbg.h>
#endif

#ifdef ANDROID
#include <android_native_app_glue.h>
#endif

using namespace CGE;

TR_CHANNEL(CGE_Engine);

void (*internalEngineMain)(int argc, char** argv) = NULL;

Engine* Engine::eng = NULL;

Engine::Engine(){
  TR_USE(CGE_Engine);
  win_ = NULL;
  rend_ = NULL;
  isUp_ = false;
  frameInterval_ = 0;
  frameTime_ = 0;
  frameInterval_ = 0;
  lastTime_ = 0;
  fpscounter_ = 0;
  fps_ = 0;
  input_ = NULL;
  console_ = NULL;
  clear_ = false;
  //guitex_ = Mutex();
  mSimulator = NULL;
  mShutdownRequested = false;
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
  maxFramerate_ = CGE::Script::instance()->getNumberSetting("timeScheme");
  bool exists;
  graphics_ = Script::instance()->getBoolSetting("graphics", &exists);
  if (!exists)
    graphics_ = true;
  physics_ = Script::instance()->getBoolSetting("physics");

  if (graphics_){
    if (!startupRenderer(type)){
      EXIT2("No valid renderer specified in engine.ini");
    }

#ifdef WIN32
    win_ = new WindowsWindow(rend_);
#endif
#if defined(LINUX) && !defined(NO_X11)
    win_ = new X11Window(rend_);
#endif
#ifdef ANDROID
  win_ = new AndroidWindow(rend_);
#endif
#if defined(QNX) || defined(NO_X11)
		win_ = new QNXWindow(rend_);
#endif
  }
  else{
    rend_ = new CGE::DummyRenderer();
  }
  if (graphics_)
    win_->init("Captains Game Engine");
  Input::Keyboard::init();
  Input::Mouse::init(graphics_);

  if (physics_){
    float step = Script::instance()->getRealSetting("integration_interval")/1000;
    mSimulator = new CGE::Simulator(step);
  }
  
  if (internalEngineMain != NULL)
    internalEngineMain(argc, argv);
  
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
  frameTime_ = getTime();
  //isUp_ = true;
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
    rend_->deinitRendering();
    TR_INFO("engine shutting down");
    isUp_ = false;
    SAFE_DELETE(mSimulator);
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
    Script::kill();
  }
  SAFE_DELETE(eng);
  TraceManager::deinit();
  //exit(0);
}

void Engine::run(){
  //handle physics, KI, ...
  if (physics_){
    mSimulator->doSimulationStep(frameInterval_);
  }

  console_->update();
  Input::Keyboard::instance()->processPressedKeys((float)frameInterval_);

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

  if (graphics_){
    //render scene
    rend_->renderScene();

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
  }

  //calculate frame rate
  double currentTime;
  if (maxFramerate_ == 0){
    currentTime = getTime();
    //cerr << currentTime;
    frameInterval_ = currentTime-frameTime_;
    frameTime_ = currentTime;
  }
  else{
    while(((currentTime = getTime())-frameTime_) <= 1.0/maxFramerate_);
    frameInterval_ = currentTime-frameTime_;
    frameTime_ = currentTime;
  }

  fpscounter_++;

  //one second passed by
  if (currentTime - lastTime_ > 1.0){
    lastTime_ = currentTime;
    fps_ = fpscounter_;
    fpscounter_ = 0;
  }
}
 

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

#ifdef UNIX
double getTime(){
  struct timespec tv;
  clock_gettime(CLOCK_MONOTONIC, &tv);
  float msec = tv.tv_sec*1000+tv.tv_nsec/1000000.0;
  return msec*0.001;
  //long factor = sysconf(_SC_CLK_TCK);
  //return times(NULL)*0.01;
}
#else
double getTime(){
  return timeGetTime()*0.001;
}
#endif

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
#include <jni.h>
#include <errno.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android_native_app_glue.h>

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 * Initialize an EGL context for the current display.
 */
 
static int engine_init_display(struct engine* engine) {
  TR_USE(CGE_Engine);
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
			EGL_DEPTH_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
    
    const EGLint surfAttribs[] ={
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, surfAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        TR_ERROR("Unable to eglMakeCurrent");
        return -1;
    }
    TR_INFO("Made egl context current");

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    return 0;
}

/**
 * Just the current frame in the display.
 */
 
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }
    glFlush();
    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                char* title = "cge";
                CGE::Engine::instance()->startup(1, &title);
                CGE::Engine::instance()->run();
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
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
            engine_draw_frame(engine);
            break;
    }
}

#endif

int Engine::mainLoop(int argc, char** argv, USERMAINFUNC engineMain, void* data){
    internalEngineMain = engineMain;
  CGE::Engine::init();
  
#ifdef WIN32
  CGE::Engine::instance()->startup(argc, argv);
  MSG msg;
  //Enter gameloop
  while (CGE::Engine::instance() != NULL && !CGE::Engine::instance()->isShutdownRequested()){
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        break;
    }
    if (CGE::Engine::instance() && !CGE::Engine::instance()->isShutdownRequested()){
      //Do not use WM_MouseMove-Event, because it causes the cursor to freeze
      POINT p;
      GetCursorPos(&p);
      ScreenToClient((HWND)static_cast<CGE::WindowsWindow*>(CGE::Engine::instance()->getWindow())->getHandle(), &p);
      Input::Mouse::instance()->move(p.x, p.y, 0);
      CGE::Engine::instance()->run();
    }
  }
  if (CGE::Engine::instance() != NULL && CGE::Engine::instance()->isShutdownRequested())
    CGE::Engine::instance()->shutdown();
  return 0;
#endif

#if defined(LINUX) && !defined(NO_X11)
  CGE::Engine::instance()->startup(argc, argv);
  XEvent event;
  bool running = true;
  CGE::X11Window* win = static_cast<CGE::X11Window*>(CGE::Engine::instance()->getWindow());
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  Display* disp = win->getDisplay();
  XSynchronize(disp, true);

  double lastClick = 0;
  while (running && CGE::Engine::instance() != NULL && !CGE::Engine::instance()->isShutdownRequested()){
    int x = XPending(disp);
    while (x > 0){
      x--;
      XNextEvent(disp, &event);
      switch (event.type){
      case Expose:
        if (event.xexpose.count != 0)
          break;
        CGE::Engine::instance()->run();
        break;
      case ConfigureNotify:
        if (event.xconfigure.width != win->getWidth() ||
          event.xconfigure.height != win->getHeight()){
          rend->resizeScene(event.xconfigure.width, event.xconfigure.height);
        }
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
          if (newClick - lastClick < 0.5){
            Input::Mouse::instance()->doubleClick(event.xbutton.x, event.xbutton.y, event.xbutton.button);
            lastClick = 0;
          }
          else{
            lastClick = getTime();
            Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, event.xbutton.button);
          }
        }
        /*
        if (event.xbutton.button == Button1)
        Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, MK_LBUTTON);
        else if (event.xbutton.button == Button2)
        Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, MK_MBUTTON);
        else if (event.xbutton.button == Button3)
        Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, MK_RBUTTON);
        */
        break;
      case ButtonRelease:
        if (event.xbutton.button <= Button3 && lastClick != 0)
          Input::Mouse::instance()->buttonUp(event.xbutton.x, event.xbutton.y, event.xbutton.button);
        break;
      case MotionNotify:
        Input::Mouse::instance()->move(event.xmotion.x, event.xmotion.y, event.xmotion.state);
        break;
      case ClientMessage:
        if (*XGetAtomName(disp, event.xclient.message_type) == *"WM_PROTOCOLS"){
          running = false;
        }
        break;
      default:
        break;
      }
    }
    CGE::Engine::instance()->run();
  }
  CGE::Engine::instance()->shutdown();
  return 0;
#endif

#ifdef ANDROID
  struct android_app* app = (android_app*)data;
  CGE::Engine::instance()->m_app = app;
  struct engine engine;

    memset(&engine, 0, sizeof(engine));
    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    engine.app = app;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            app->looper, LOOPER_ID_USER, NULL, NULL);

    if (app->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)app->savedState;
    }

//ANativeActivity_finish(app->activity);
//CGE::Engine::instance()->startup(argc, argv);
while (1){
  TR_USE(CGE_Engine);
  int ident;
  int events;
  struct android_poll_source* source;
  while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            // Process this event.
            if (source != NULL) {
                source->process(app, source);
            }
  
     // If a sensor has data, process it now.
      if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                            &event, 1) > 0) {
                        TR_DEBUG("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y,
                                event.acceleration.z);
                    }
                }
            }
    if (app->destroyRequested){
      engine_term_display(&engine);
      CGE::Engine::instance()->shutdown();
      return 0;
    }
    
    if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            CGE::Engine::instance()->run();
            engine_draw_frame(&engine);
        }
        //CGE::Engine::instance()->run();
  }
}
  CGE::Engine::instance()->shutdown();
  return 0;
#endif
}