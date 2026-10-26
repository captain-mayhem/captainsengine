#include "engine.h"
#include "../audio/audio.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../input/touch.h"
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
#include "../window/nativeAndroid.h"
#include "../window/nativeEmscripten.h"
#include "script.h"
#include "../renderer/drawcontext.h"
#include "../renderer/font.h"
#include "../renderer/screentext.h"
#include "../renderer/forms.h"
#include "../renderer/spritebatch.h"
#include "../math/matrixstack.h"
#include "../io/Tracing.h"
#include "../io/TraceManager.h"
#include "../platform/settings.h"
#include "../platform/platform_internal.h"

#include <algorithm>
#include <cstring>
#include <atomic>
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
#include <X11/keysym.h>
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
//! ImGui's font size at a content scale of 1 (its own font has 13 pixels)
static const float IMGUI_FONT_SIZE = 15.0f;

Engine::Engine(){
  TR_USE(CGE_Engine);
  isUp_ = false;
  app_ = nullptr;
  paused_ = false;
  rendering_ = false;
  frameDepth_ = 0;
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
#endif
#ifdef ANDROID
  m_app = nullptr;
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

  deviceDesc_ = Gfx::DeviceDesc();
  deviceDesc_.vsync = Script::instance()->getBoolSetting("vsync", &exists) || !exists;
  int bpp = Script::instance()->getNumberSetting("bitsPerPixel");
  if (bpp > 0)
    deviceDesc_.colorBits = bpp;
#ifndef NDEBUG
  deviceDesc_.debug = true;
#endif

  if (graphics_){
    device_ = Gfx::RenderDevice::create(type);
    if (!device_){
      EXIT2("The render backend %s is not available", type.c_str());
      return;
    }

#ifdef WIN32
    win_ = std::make_unique<WindowsWindow>(device_.get(), deviceDesc_);
#endif
#if defined(LINUX)
    win_ = std::make_unique<X11Window>(device_.get(), deviceDesc_);
#endif
#ifdef ANDROID
    win_ = std::make_unique<AndroidWindow>(device_.get(), deviceDesc_);
#endif
#ifdef __EMSCRIPTEN__
    win_ = std::make_unique<EmscriptenWindow>(device_.get(), deviceDesc_);
#endif
  }
  else{
    device_ = Gfx::RenderDevice::create("Null");
    device_->init(Gfx::NativeWindow(), deviceDesc_);
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
    if (!device_->init(win_->getNativeWindow(), deviceDesc_)){
      EXIT2("The render device could not be initialised");
      win_->kill();
      return;
    }
    win_->setWidth(device_->getWidth());
    win_->setHeight(device_->getHeight());
  }
  for (auto& text : texts_)
    text = std::make_unique<CGE::ScreenText>();
  createDeviceObjects();
  Input::Keyboard::init();
  Input::Mouse::init(graphics_);
  Input::Touch::init();
  //the first finger is the left mouse button unless the application handles touches itself
  bool touchAsMouse = Script::instance()->getBoolSetting("touchAsMouse", &exists);
  Input::Touch::instance()->setMouseEmulation(touchAsMouse || !exists);
  
  if (graphics_)
    startupImGui();
  console_ = std::make_unique<CGE::Console>();
  //on the web it is opened here, on the browser's main thread, as it must be
  audio_ = std::make_unique<CGE::Audio>();
  bool audio = Script::instance()->getBoolSetting("audio", &exists);
  if (audio || !exists)
    audio_->init();
  isUp_ = true;
  app_ = internalApplication;
  if (app_)
    app_->init(argc, argv);
  frameTime_ = getTime();
  updateLag_ = 0;
}

void Engine::createDeviceObjects(){
  draw_ = std::make_unique<Gfx::DrawContext>(*device_);
  if (!graphics_)
    return;
  forms_ = std::make_unique<CGE::Forms>(*device_);
  sprites_ = std::make_unique<Gfx::SpriteBatch>(*draw_);
}

void Engine::destroyDeviceObjects(){
  //the texts stay, their fonts are loaded again
  for (auto& text : texts_){
    if (text)
      text->releaseFonts();
  }
  forms_.reset();
  sprites_.reset();
  draw_.reset();
}

void Engine::restartRenderDevice(){
  TR_USE(CGE_Engine);
  if (!isUp_ || !graphics_ || rendering_)
    return;
  TR_INFO("Creating the render device and its resources again");
  if (app_)
    app_->onDeviceLost();
  if (imgui_)
    ImGui_ImplCGE_Render_Shutdown();
  destroyDeviceObjects();
  device_->shutdown();
  if (!device_->init(win_->getNativeWindow(), deviceDesc_)){
    EXIT2("The render device could not be initialised again");
    return;
  }
  createDeviceObjects();
  //ImGui creates its textures again by itself
  if (imgui_ && !ImGui_ImplCGE_Render_Init(*device_))
    TR_WARN("ImGui can't draw on this render device");
  if (app_)
    app_->onDeviceRestored();
  //the new context's drawable may have another size
  if (device_->getWidth() != win_->getWidth() || device_->getHeight() != win_->getHeight())
    resize(device_->getWidth(), device_->getHeight());
}

void Engine::startupImGui(){
  TR_USE(CGE_Engine);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  //applications that want ImGui's window settings saved set a file name
  io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  //the engine's font through FreeType (imgui_freetype), which renders the
  //glyphs a text needs when it is first drawn; ImGui's own font if it's missing
  std::vector<uint8> font;
  if (CGE::Font::loadDefaultData(font)){
    void* data = IM_ALLOC(font.size());
    memcpy(data, font.data(), font.size());
    io.Fonts->AddFontFromMemoryTTF(data, (int)font.size(), IMGUI_FONT_SIZE);
  }
  else
    TR_WARN("ImGui uses its own font: %s is missing", CGE::Font::getDefaultPath());
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
    //after the application, which deletes its players and streams
    audio_.reset();
    Input::Keyboard::release();
    Input::Mouse::release();
    Input::Touch::release();
    console_.reset();
    //its textures are on the device
    shutdownImGui();
    destroyDeviceObjects();
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
#ifndef ANDROID
  TraceManager::deinit();
#else
  //the process goes on and may start the engine again for a new activity;
  //the trace channels registered with the manager only once
#endif
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

void Engine::runModalFrame(){
  if (frameDepth_ == 0)
    frame(true);
}

void Engine::frame(bool update){
  if (mShutdownRequested || !isUp_)
    return;
  struct Depth{
    int& depth;
    Depth(int& d) : depth(d) {++depth;}
    ~Depth() {--depth;}
  } depth(frameDepth_);
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
  audio_->update();

  if (paused_){
#ifndef __EMSCRIPTEN__
    //nothing to do in the background (the browser sends no frames then)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
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
  //Android: the context and the resources were lost
  if (graphics_ && device_->isLost() && !mShutdownRequested)
    restartRenderDevice();
#ifdef __EMSCRIPTEN__
  //a page may be closed at any time without a shutdown, and the IndexedDB
  //writes of a closing page don't finish, so changes are saved right away
  Settings::saveUser();
#endif
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

  renderScreenTexts();

  //ImGui on top of everything, the console last
  if (imgui_){
    console_->render();
    renderImGui();
  }

  device_->endFrame();
  rendering_ = false;
}

void Engine::renderScreenTexts(){
  bool texts = false;
  for (auto& text : texts_)
    texts = texts || !text->isEmpty();
  if (!texts)
    return;
  Gfx::PassDesc pass;
  pass.colorLoad = Gfx::LoadAction::Load;
  pass.depthLoad = Gfx::LoadAction::Load;
  if (!device_->beginPass(pass))
    return;
  //in the virtual screen with y down, over the frame
  VirtualScreen& screen = getVirtualScreen();
  draw_->setProjection(screen.ortho(*device_, true));
  draw_->setView(Matrix(Matrix::Identity));
  draw_->model().load(Matrix(Matrix::Identity));
  draw_->setDepth(false, false);
  Gfx::BlendMode blend = sprites_->getBlend();
  sprites_->setBlend(Gfx::BlendMode::Alpha);
  for (auto& text : texts_)
    text->render(*sprites_, (float)screen.getHeight(), frameInterval_);
  sprites_->flush();
  sprites_->setBlend(blend);
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
#ifdef __EMSCRIPTEN__
  //the browser paces the frames (requestAnimationFrame), and sleeping would
  //block its main thread
  return;
#endif
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
  //nothing plays in the background
  if (audio_)
    audio_->pause();
  //the system may end the application in the background (Android)
  Settings::saveUser();
}

void Engine::resume(){
  if (!paused_)
    return;
  paused_ = false;
  if (audio_)
    audio_->resume();
  //don't catch up with the time spent in the background
  frameTime_ = getTime();
  updateLag_ = 0;
  if (app_)
    app_->onResume();
}

#ifdef WIN32
void Engine::processEvents(){
  MSG msg;
  //the W functions, so that a Unicode window's WM_CHAR stays UTF-16
  while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
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

//! the characters a key press types, through the window's input method
void typeCharacters(CGE::X11Window* win, XKeyEvent& event){
  XIC ic = win->getInputContext();
  if (!ic){
    //Latin-1 without an input method
    char ch;
    if (XLookupString(&event, &ch, 1, nullptr, nullptr) == 1)
      Input::Keyboard::instance()->character((unsigned char)ch);
    return;
  }
  //Status is undefined by imgui.h
  int status;
  char buffer[64];
  int length = Xutf8LookupString(ic, &event, buffer, sizeof(buffer), nullptr, &status);
  if (status == XBufferOverflow){
    std::string text(length, '\0');
    length = Xutf8LookupString(ic, &event, &text[0], length, nullptr, &status);
    if (status == XLookupChars || status == XLookupBoth)
      Input::Keyboard::instance()->text(text.c_str(), length);
  }
  else if (status == XLookupChars || status == XLookupBoth)
    Input::Keyboard::instance()->text(buffer, length);
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
    //the input method's events, and the keys it composes characters from:
    //those are still keys
    bool filtered = XFilterEvent(&event, None) == True;
    if (filtered && event.type != KeyPress && event.type != KeyRelease)
      continue;
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
      //Alt+Enter toggles fullscreen
      if (XLookupKeysym(&event.xkey, 0) == XK_Return && (event.xkey.state & Mod1Mask)){
        win->setFullscreen(!win->isFullscreen());
        break;
      }
      //the key without modifiers, as for its release (Shift+A is KEY_A, too)
      KeySym key = XLookupKeysym(&event.xkey, 0);
      imguiModifiers(event.xkey.state);
      ImGui_ImplCGE_Platform_AddKey((int)key, true);
      Input::Keyboard::instance()->keyDown(key);
      //a key the input method took (e.g. a dead key) types nothing yet
      if (!filtered)
        typeCharacters(win, event.xkey);
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
        Input::Mouse::instance()->press(event.xbutton.x, event.xbutton.y, event.xbutton.button);
      }
      break;
    case ButtonRelease:
      imguiButton(event.xbutton, false);
      if (event.xbutton.button <= Button3)
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
      if (win->getInputContext())
        XSetICFocus(win->getInputContext());
      ImGui_ImplCGE_Platform_AddFocus(true);
      break;
    case FocusOut:
      if (win->getInputContext())
        XUnsetICFocus(win->getInputContext());
      ImGui_ImplCGE_Platform_AddFocus(false);
      break;
    case ClientMessage:
      if (win->isCloseRequest(event))
        requestShutdown();
      break;
    //other clients asking for the clipboard's text, or taking it over
    case SelectionRequest:
    case SelectionClear:
      win->handleSelectionEvent(event);
      break;
    default:
      break;
    }
  }
}
#endif

#ifdef ANDROID
void Engine::processEvents(){
  //the activity's commands (handleCmd) and input events (AndroidWindow)
  android_app* app = m_app;
  int events;
  android_poll_source* source;
  while (ALooper_pollOnce(0, nullptr, &events, (void**)&source) >= 0){
    if (source)
      source->process(app, source);
    if (app->destroyRequested)
      return;
  }
  AndroidWindow::dispatchActivityEvents();
  //a rotation or a split screen changes the window's size without a command
  //that comes reliably after the change
  if (win_ && !device_->isWindowReleased() && static_cast<AndroidWindow*>(win_.get())->updateSize())
    resize(win_->getWidth(), win_->getHeight());
}

namespace{

//! the state of an activity (android_app::userData)
struct ActivityState{
  //! between onResume and onPause
  bool resumed = false;
  //! the window has the input focus
  bool focused = false;
};

//! the activity whose thread runs the engine, nullptr while none does
/*! The system can create a new activity of the process before the old one
    is destroyed (e.g. when it is started with FLAG_ACTIVITY_CLEAR_TASK);
    the new one waits until the old one shut the engine down. */
std::atomic<android_app*> engineActivity(nullptr);

}

void Engine::handleCmd(struct android_app* app, int32_t cmd){
  TR_USE(CGE_Engine);
  ActivityState* activity = (ActivityState*)app->userData;
  switch (cmd){
    case APP_CMD_RESUME: activity->resumed = true; break;
    case APP_CMD_PAUSE: activity->resumed = false; break;
    case APP_CMD_GAINED_FOCUS: activity->focused = true; break;
    case APP_CMD_LOST_FOCUS: activity->focused = false; break;
    default: break;
  }
  //nothing else to do while another activity has the engine, or after the shutdown
  Engine* engine = engineActivity == app ? Engine::instance() : nullptr;
  if (!engine)
    return;
  switch (cmd){
    case APP_CMD_INIT_WINDOW:
      engine->windowCreated();
      break;
    case APP_CMD_TERM_WINDOW:
      //the window goes away when this returns
      if (engine->isUp_ && engine->graphics_)
        engine->device_->releaseWindow();
      break;
    case APP_CMD_SAVE_STATE:
      //the system may end the process in the background
      if (engine->isUp_)
        Settings::saveUser();
      break;
    case APP_CMD_DESTROY:
      TR_INFO("The activity is destroyed");
      engine->requestShutdown();
      break;
    default:
      break;
  }
  engine->updateActivity();
}

void Engine::windowCreated(){
  if (!m_app->window || mShutdownRequested)
    return;
  if (!isUp_){
    //argv[0], then the arguments of the activity's intent
    std::vector<std::string> args = PlatformInternal::getActivityArguments();
    args.insert(args.begin(), "cge");
    std::vector<char*> argv;
    for (std::string& arg : args)
      argv.push_back(&arg[0]);
    argv.push_back(nullptr);
    startup((int)args.size(), argv.data());
  }
  else if (graphics_){
    //the next window; the resources are still there unless the context was lost
    if (!device_->attachWindow(win_->getNativeWindow()))
      restartRenderDevice();
    if (static_cast<AndroidWindow*>(win_.get())->updateSize() ||
      device_->getWidth() != win_->getWidth() || device_->getHeight() != win_->getHeight())
      resize(win_->getWidth(), win_->getHeight());
  }
}

void Engine::updateActivity(){
  if (!isUp_)
    return;
  ActivityState* activity = (ActivityState*)m_app->userData;
  bool window = !graphics_ || (m_app->window && !device_->isWindowReleased());
  if (activity->resumed && activity->focused && window)
    resume();
  else
    pause();
}
#endif

#ifdef __EMSCRIPTEN__
void Engine::processEvents(){
  //the browser delivers the input events between frames (nativeEmscripten.cpp);
  //the canvas's size is checked here, since not every change sends an event
  if (!win_)
    return;
  if (static_cast<EmscriptenWindow*>(win_.get())->updateSize())
    resize(win_->getWidth(), win_->getHeight());
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

IApplication::~IApplication(){
}

void IApplication::onKeyDown(int key){
  if (key == KEY_ESCAPE)
    EXIT();
}

int CGE::runApplication(int argc, char** argv, IApplication& app, void* data){
#if !defined(ANDROID)
  internalApplication = &app;
  PlatformInternal::init(argc, argv, data);
  CGE::Engine::init();
#endif

#if defined(__EMSCRIPTEN__)
  installBrowserConsoleTracing();
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
  //the browser calls runFrame once per frame; this doesn't return, so the
  //application in main's stack frame (CGE_MAIN) stays valid
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
  }, 0, true);
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
  //the engine starts when the activity's window comes (handleCmd) and stops
  //when the application quits or the activity is destroyed
  android_app* state = (android_app*)data;
  ActivityState activity;
  state->userData = &activity;
  state->onAppCmd = Engine::handleCmd;
  state->onInputEvent = AndroidWindow::handleInput;
  bool started = false;
  bool finished = false;
  while (!state->destroyRequested){
    if (!started){
      //another activity of the process may still be shutting the engine down
      android_app* none = nullptr;
      if (engineActivity.compare_exchange_strong(none, state)){
        started = true;
        installLogcatTracing();
        internalApplication = &app;
        PlatformInternal::init(argc, argv, state);
        CGE::Engine::init();
        CGE::Engine::instance()->m_app = state;
        AndroidWindow::registerNatives(state);
        //the window may have come already
        CGE::Engine::instance()->windowCreated();
        CGE::Engine::instance()->updateActivity();
        continue;
      }
      int events;
      android_poll_source* source;
      if (ALooper_pollOnce(20, nullptr, &events, (void**)&source) >= 0 && source)
        source->process(state, source);
      continue;
    }
    Engine* engine = finished ? nullptr : CGE::Engine::instance();
    if (engine && engine->isUp_ && !engine->paused_ && !engine->mShutdownRequested){
      //processes the events itself
      engine->runFrame();
    }
    else{
      //nothing to draw: wait for the next event
      int events;
      android_poll_source* source;
      if (ALooper_pollOnce(-1, nullptr, &events, (void**)&source) >= 0 && source)
        source->process(state, source);
      if (engine)
        AndroidWindow::dispatchActivityEvents();
    }
    if (engine && engine->mShutdownRequested){
      //the application quit, or the activity is going: shut down now, while
      //the window is still there, and wait for the activity's end
      engine->shutdown();
      internalApplication = nullptr;
      finished = true;
      engineActivity = nullptr;
      if (!state->destroyRequested)
        ANativeActivity_finish(state->activity);
    }
  }
  if (started && !finished){
    CGE::Engine::instance()->shutdown();
    internalApplication = nullptr;
    engineActivity = nullptr;
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
