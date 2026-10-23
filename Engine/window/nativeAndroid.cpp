#if defined(ANDROID)
#include <atomic>

#include <jni.h>
#include <android/configuration.h>
#include <android/log.h>
#include <android/input.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>

#include "../system/engine.h"
#include "../io/TraceManager.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "../ui/imgui_impl_cge_platform.h"
#include "nativeAndroid.h"

using namespace CGE;

TR_CHANNEL_EXT(CGE_Window);

namespace{

//! the time of the last press, 0 after a double click (as on X11)
double lastClick = 0;
//! the buttons held down (MB_ bits)
int buttonState = 0;
//! the pointer id of the finger that acts as the mouse, -1 for none
int32_t mouseTouch = -1;
//! the mouse buttons held down (AMOTION_EVENT_BUTTON_ bits)
int32_t mouseButtons = 0;
//! back gestures reported by CgeActivity, not yet dispatched
std::atomic<int> backPresses(0);
//! the looper of the thread that runs the engine, woken up by back gestures
ALooper* engineLooper = nullptr;

int toImGuiButton(int button){
  switch (button){
    case MB_LEFT: return ImGuiMouseButton_Left;
    case MB_RIGHT: return ImGuiMouseButton_Right;
    default: return ImGuiMouseButton_Middle;
  }
}

void press(int x, int y, int button){
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  ImGui_ImplCGE_Platform_AddMouseButton(toImGuiButton(button), true);
  buttonState |= button;
  double now = getTime();
  if (now - lastClick < 0.5){
    Input::Mouse::instance()->doubleClick(x, y, button);
    lastClick = 0;
  }
  else{
    lastClick = now;
    Input::Mouse::instance()->buttonDown(x, y, button);
  }
}

void release(int x, int y, int button){
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  ImGui_ImplCGE_Platform_AddMouseButton(toImGuiButton(button), false);
  buttonState &= ~button;
  //the release of a double click doesn't count
  if (lastClick != 0)
    Input::Mouse::instance()->buttonUp(x, y, button);
}

void move(int x, int y){
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  Input::Mouse::instance()->move(x, y, buttonState);
}

//! the index of a pointer in a motion event, -1 if it isn't there
int pointerIndex(const AInputEvent* event, int32_t id){
  size_t count = AMotionEvent_getPointerCount(event);
  for (size_t i = 0; i < count; ++i){
    if (AMotionEvent_getPointerId(event, i) == id)
      return (int)i;
  }
  return -1;
}

//! a finger on the touch screen; the first one is the left mouse button
//! (touch events of their own come with Phase 1c step 4)
void touchEvent(const AInputEvent* event){
  int32_t action = AMotionEvent_getAction(event);
  int32_t masked = action & AMOTION_EVENT_ACTION_MASK;
  size_t index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
  switch (masked){
    case AMOTION_EVENT_ACTION_DOWN:
      mouseTouch = AMotionEvent_getPointerId(event, 0);
      press((int)AMotionEvent_getX(event, 0), (int)AMotionEvent_getY(event, 0), MB_LEFT);
      break;
    case AMOTION_EVENT_ACTION_MOVE:{
      int i = pointerIndex(event, mouseTouch);
      if (i >= 0)
        move((int)AMotionEvent_getX(event, i), (int)AMotionEvent_getY(event, i));
      break;
    }
    case AMOTION_EVENT_ACTION_POINTER_UP:
      if (AMotionEvent_getPointerId(event, index) != mouseTouch)
        break;
      release((int)AMotionEvent_getX(event, index), (int)AMotionEvent_getY(event, index), MB_LEFT);
      mouseTouch = -1;
      break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL:{
      int i = pointerIndex(event, mouseTouch);
      if (i >= 0)
        release((int)AMotionEvent_getX(event, i), (int)AMotionEvent_getY(event, i), MB_LEFT);
      mouseTouch = -1;
      break;
    }
    default:
      break;
  }
}

//! a mouse (or a touchpad acting as one); the buttons come from the button
//! state, because the event's action button needs API 33
void mouseEvent(const AInputEvent* event){
  int x = (int)AMotionEvent_getX(event, 0);
  int y = (int)AMotionEvent_getY(event, 0);
  int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
  if (action == AMOTION_EVENT_ACTION_SCROLL){
    float wheel = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);
    float hwheel = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HSCROLL, 0);
    ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
    ImGui_ImplCGE_Platform_AddMouseWheel(-hwheel, wheel);
    if (wheel != 0)
      Input::Mouse::instance()->wheel(x, y, wheel > 0 ? 1 : -1);
    return;
  }
  if (action == AMOTION_EVENT_ACTION_MOVE || action == AMOTION_EVENT_ACTION_HOVER_MOVE)
    move(x, y);
  static const int32_t buttons[] = {AMOTION_EVENT_BUTTON_PRIMARY, AMOTION_EVENT_BUTTON_SECONDARY, AMOTION_EVENT_BUTTON_TERTIARY};
  static const int mbs[] = {MB_LEFT, MB_RIGHT, MB_MIDDLE};
  int32_t state = AMotionEvent_getButtonState(event);
  for (int i = 0; i < 3; ++i){
    bool down = (state & buttons[i]) != 0;
    if (down == ((mouseButtons & buttons[i]) != 0))
      continue;
    if (down)
      press(x, y, mbs[i]);
    else
      release(x, y, mbs[i]);
  }
  mouseButtons = state;
}

//! the character a key types without a keyboard layout: letters, digits
//! and space (all of Unicode comes with Phase 1c step 4); 0 for none
unsigned char keyChar(int32_t code, int32_t meta){
  if (meta & (AMETA_CTRL_ON | AMETA_ALT_ON | AMETA_META_ON))
    return 0;
  if (code >= AKEYCODE_A && code <= AKEYCODE_Z){
    bool upper = ((meta & AMETA_SHIFT_ON) != 0) != ((meta & AMETA_CAPS_LOCK_ON) != 0);
    return (unsigned char)((upper ? 'A' : 'a') + (code - AKEYCODE_A));
  }
  if (code >= AKEYCODE_0 && code <= AKEYCODE_9 && !(meta & AMETA_SHIFT_ON))
    return (unsigned char)('0' + (code - AKEYCODE_0));
  if (code == AKEYCODE_SPACE)
    return ' ';
  return 0;
}

void keyEvent(const AInputEvent* event){
  int32_t code = AKeyEvent_getKeyCode(event);
  int32_t meta = AKeyEvent_getMetaState(event);
  //the back button opens the application's menu, like Escape
  if (code == AKEYCODE_BACK)
    code = KEY_ESCAPE;
  ImGui_ImplCGE_Platform_AddModifiers((meta & AMETA_CTRL_ON) != 0, (meta & AMETA_SHIFT_ON) != 0,
    (meta & AMETA_ALT_ON) != 0, (meta & AMETA_META_ON) != 0);
  switch (AKeyEvent_getAction(event)){
    case AKEY_EVENT_ACTION_DOWN:{
      ImGui_ImplCGE_Platform_AddKey(code, true);
      Input::Keyboard::instance()->keyDown(code);
      unsigned char ch = keyChar(code, meta);
      if (ch)
        Input::Keyboard::instance()->ascii(ch);
      break;
    }
    case AKEY_EVENT_ACTION_UP:
      ImGui_ImplCGE_Platform_AddKey(code, false);
      Input::Keyboard::instance()->keyUp(code);
      break;
    default:
      break;
  }
}

//! the keys the system handles: volume, media, calls, the camera and the like
bool isSystemKey(int32_t code){
  switch (code){
    case AKEYCODE_VOLUME_UP:
    case AKEYCODE_VOLUME_DOWN:
    case AKEYCODE_VOLUME_MUTE:
    case AKEYCODE_MUTE:
    case AKEYCODE_POWER:
    case AKEYCODE_HOME:
    case AKEYCODE_APP_SWITCH:
    case AKEYCODE_CALL:
    case AKEYCODE_ENDCALL:
    case AKEYCODE_CAMERA:
    case AKEYCODE_FOCUS:
    case AKEYCODE_HEADSETHOOK:
    case AKEYCODE_MEDIA_PLAY_PAUSE:
    case AKEYCODE_MEDIA_PLAY:
    case AKEYCODE_MEDIA_PAUSE:
    case AKEYCODE_MEDIA_STOP:
    case AKEYCODE_MEDIA_NEXT:
    case AKEYCODE_MEDIA_PREVIOUS:
    case AKEYCODE_SEARCH:
    case AKEYCODE_ASSIST:
      return true;
    default:
      return false;
  }
}

//! CgeActivity.nativeBack: called on the UI thread for each back gesture
void JNICALL nativeBack(JNIEnv*, jclass){
  backPresses++;
  if (engineLooper)
    ALooper_wake(engineLooper);
}

//! the traces as logcat messages of the tag CGE
class LogcatOutputter : public TraceOutputter{
public:
  bool init() override {return true;}
  void trace(unsigned channel, int level, const char* function, const char* message) override{
    int prio;
    switch (level){
      case TRACE_FATAL_ERROR: prio = ANDROID_LOG_FATAL; break;
      case TRACE_ERROR: prio = ANDROID_LOG_ERROR; break;
      case TRACE_WARNING: prio = ANDROID_LOG_WARN; break;
      case TRACE_INFO: prio = ANDROID_LOG_INFO; break;
      case TRACE_DEBUG: prio = ANDROID_LOG_DEBUG; break;
      default: prio = ANDROID_LOG_VERBOSE; break;
    }
    __android_log_print(prio, "CGE", "%s: %s", function, message);
  }
};

//! is the engine running and taking input? With an app, only from that
//! activity (an old one may still be going while a new one starts)
bool engineTakesInput(android_app* app = nullptr){
  Engine* engine = Engine::instance();
  return engine && (!app || engine->getApp() == app) && engine->getWindow() && Input::Mouse::instance() && Input::Keyboard::instance();
}

}

AndroidWindow::AndroidWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc){
}

void AndroidWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");
  lastClick = 0;
  buttonState = 0;
  mouseTouch = -1;
  mouseButtons = 0;
  updateSize();
}

void AndroidWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");
}

Gfx::NativeWindow AndroidWindow::getNativeWindow() const{
  Gfx::NativeWindow wnd;
  wnd.kind = Gfx::NativeWindow::Android;
  wnd.window = (uintptr_t)CGE::Engine::instance()->getApp()->window;
  return wnd;
}

float AndroidWindow::getContentScale() const{
  android_app* app = Engine::instance()->getApp();
  int32_t density = app->config ? AConfiguration_getDensity(app->config) : ACONFIGURATION_DENSITY_DEFAULT;
  if (density == ACONFIGURATION_DENSITY_DEFAULT || density == ACONFIGURATION_DENSITY_NONE || density == ACONFIGURATION_DENSITY_ANY)
    return 1;
  return density / (float)ACONFIGURATION_DENSITY_MEDIUM;
}

bool AndroidWindow::updateSize(){
  ANativeWindow* wnd = Engine::instance()->getApp()->window;
  if (!wnd)
    return false;
  int width = ANativeWindow_getWidth(wnd);
  int height = ANativeWindow_getHeight(wnd);
  if (width <= 0 || height <= 0 || (width == width_ && height == height_))
    return false;
  width_ = width;
  height_ = height;
  return true;
}

int32_t AndroidWindow::handleInput(android_app* app, AInputEvent* event){
  if (!engineTakesInput(app))
    return 0;
  switch (AInputEvent_getType(event)){
    case AINPUT_EVENT_TYPE_KEY:
      if (isSystemKey(AKeyEvent_getKeyCode(event)))
        return 0;
      keyEvent(event);
      return 1;
    case AINPUT_EVENT_TYPE_MOTION:{
      int32_t source = AInputEvent_getSource(event);
      if ((source & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE)
        mouseEvent(event);
      else if ((source & AINPUT_SOURCE_CLASS_POINTER) != 0)
        touchEvent(event);
      else
        return 0;
      return 1;
    }
    default:
      return 0;
  }
}

void AndroidWindow::dispatchBack(){
  while (backPresses > 0){
    backPresses--;
    if (!engineTakesInput())
      continue;
    ImGui_ImplCGE_Platform_AddKey(KEY_ESCAPE, true);
    Input::Keyboard::instance()->keyDown(KEY_ESCAPE);
    ImGui_ImplCGE_Platform_AddKey(KEY_ESCAPE, false);
    Input::Keyboard::instance()->keyUp(KEY_ESCAPE);
  }
}

void AndroidWindow::registerBackCallback(android_app* app){
  TR_USE(CGE_Window);
  engineLooper = app->looper;
  backPresses = 0;
  JNIEnv* env = nullptr;
  if (app->activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK){
    TR_WARN("Can't attach to the Java VM");
    return;
  }
  //CgeActivity through the activity's class loader, which knows the app's
  //classes (FindClass on this thread only knows the system's)
  jclass activityClass = env->GetObjectClass(app->activity->clazz);
  jmethodID getClassLoader = env->GetMethodID(activityClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
  jobject loader = env->CallObjectMethod(app->activity->clazz, getClassLoader);
  jclass loaderClass = env->FindClass("java/lang/ClassLoader");
  jmethodID loadClass = env->GetMethodID(loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
  jstring name = env->NewStringUTF("de.captain.cge.CgeActivity");
  jclass cgeActivity = (jclass)env->CallObjectMethod(loader, loadClass, name);
  if (env->ExceptionCheck()){
    env->ExceptionClear();
    TR_INFO("The activity isn't a CgeActivity: back is only a key");
  }
  else if (cgeActivity){
    JNINativeMethod method = {"nativeBack", "()V", (void*)nativeBack};
    if (env->RegisterNatives(cgeActivity, &method, 1) != JNI_OK){
      env->ExceptionClear();
      TR_WARN("Can't register CgeActivity.nativeBack");
    }
    env->DeleteLocalRef(cgeActivity);
  }
  env->DeleteLocalRef(name);
  env->DeleteLocalRef(loaderClass);
  env->DeleteLocalRef(loader);
  env->DeleteLocalRef(activityClass);
  //the registration stays
  app->activity->vm->DetachCurrentThread();
}

void CGE::installLogcatTracing(){
  //once per process, which may start the engine for several activities
  static bool installed = false;
  if (installed)
    return;
  installed = true;
  TraceManager::instance()->setTraceOutputter(new LogcatOutputter());
}

#endif
