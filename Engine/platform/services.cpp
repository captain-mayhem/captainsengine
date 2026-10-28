#include "services.h"

#include <chrono>
#include <cstdlib>

#include "../io/Tracing.h"
#include "../system/engine.h"
#include "../input/mouse.h"
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
#include "../window/nativeEmscripten.h"
#include "assets.h"
#include "userdirs.h"
#include "platform_internal.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif
#if defined(LINUX)
#include <X11/cursorfont.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#ifdef ANDROID
#include <android_native_app_glue.h>
#include <jni.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// The browser's clipboard (guit3's approach): a page can't read it
// synchronously, so a copy of its text is kept in Module.cgeClipboard. A copy
// sets it and writes the system clipboard (navigator.clipboard, or a copy
// event where that is missing, e.g. on plain HTTP); both need the user
// activation of the key press or click that copies. Ctrl+V's default action
// is a paste event with the clipboard's text, which updates the copy before
// the next frame handles the key (the window keeps the default of
// Ctrl+C/V/X). Without such an event, only the last copied or pasted text
// can be pasted.
EM_JS_DEPS(cge_clipboard, "$stringToNewUTF8,$UTF8ToString");

EM_JS(void, cge_clipboard_install, (), {
  if (typeof document === "undefined" || Module["cgeClipboard"] !== undefined)
    return;
  Module["cgeClipboard"] = "";
  document.addEventListener("paste", function(e){
    if (e.clipboardData)
      Module["cgeClipboard"] = e.clipboardData.getData("text/plain");
  }, true);
});

EM_JS(void, cge_clipboard_set, (const char* text), {
  var s = UTF8ToString(text);
  Module["cgeClipboard"] = s;
  if (navigator.clipboard && navigator.clipboard.writeText){
    navigator.clipboard.writeText(s).catch(function(err){
      console.warn("CGE: can't write the clipboard: " + err);
    });
    return;
  }
  var onCopy = function(e){
    e.clipboardData.setData("text/plain", s);
    e.preventDefault();
  };
  document.addEventListener("copy", onCopy, true);
  try{
    document.execCommand("copy");
  }
  finally{
    document.removeEventListener("copy", onCopy, true);
  }
});

EM_JS(char*, cge_clipboard_get, (), {
  var s = Module["cgeClipboard"];
  return s ? stringToNewUTF8(s) : 0;
});

EM_JS(int, cge_clipboard_has_text, (), {
  return Module["cgeClipboard"] ? 1 : 0;
});

//! sets the CSS cursor of the canvas
EM_JS(void, cge_set_canvas_cursor, (const char* canvas, const char* cursor), {
  var element = document.querySelector(UTF8ToString(canvas));
  if (element)
    element.style.cursor = UTF8ToString(cursor);
});
#endif

using namespace CGE;

TR_CHANNEL(CGE_Platform);

namespace{

Platform::Cursor currentCursor = Platform::CURSOR_ARROW;
//! the cursor ImGui wants while the mouse is over one of its windows, or -1
int overrideCursor = -1;
//! between startTextInput and stopTextInput
bool textInputActive = false;

//! the cursor shown over the window
Platform::Cursor shownCursor(){
  return overrideCursor >= 0 ? (Platform::Cursor)overrideCursor : currentCursor;
}
#ifdef ANDROID
ANativeActivity* activity = nullptr;
#endif

#ifdef __EMSCRIPTEN__
EmscriptenWindow* engineWindow(){
  Engine* engine = Engine::instance();
  return engine ? dynamic_cast<EmscriptenWindow*>(engine->getWindow()) : nullptr;
}

//! the CSS name of a cursor
const char* cssCursor(Platform::Cursor cursor){
  switch (cursor){
    case Platform::CURSOR_IBEAM: return "text";
    case Platform::CURSOR_HAND: return "pointer";
    case Platform::CURSOR_WAIT: return "wait";
    case Platform::CURSOR_CROSSHAIR: return "crosshair";
    case Platform::CURSOR_RESIZE_H: return "ew-resize";
    case Platform::CURSOR_RESIZE_V: return "ns-resize";
    case Platform::CURSOR_MOVE: return "move";
    case Platform::CURSOR_FORBIDDEN: return "not-allowed";
    default: return "default";
  }
}
#endif

#ifdef WIN32
HWND engineWindow(){
  Engine* engine = Engine::instance();
  WindowsWindow* wnd = engine ? dynamic_cast<WindowsWindow*>(engine->getWindow()) : nullptr;
  return wnd ? (HWND)wnd->getHandle() : nullptr;
}

std::wstring toWide(const std::string& utf8){
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
  std::wstring ret(len, L'\0');
  if (len > 0)
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &ret[0], len);
  return ret;
}

std::string toUtf8(const wchar_t* wide){
  int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
  if (len <= 1)
    return std::string();
  std::string ret(len - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide, -1, &ret[0], len, nullptr, nullptr);
  return ret;
}

HCURSOR systemCursor(Platform::Cursor cursor){
  LPCTSTR id = IDC_ARROW;
  switch (cursor){
    case Platform::CURSOR_ARROW: id = IDC_ARROW; break;
    case Platform::CURSOR_IBEAM: id = IDC_IBEAM; break;
    case Platform::CURSOR_HAND: id = IDC_HAND; break;
    case Platform::CURSOR_WAIT: id = IDC_WAIT; break;
    case Platform::CURSOR_CROSSHAIR: id = IDC_CROSS; break;
    case Platform::CURSOR_RESIZE_H: id = IDC_SIZEWE; break;
    case Platform::CURSOR_RESIZE_V: id = IDC_SIZENS; break;
    case Platform::CURSOR_MOVE: id = IDC_SIZEALL; break;
    case Platform::CURSOR_FORBIDDEN: id = IDC_NO; break;
  }
  return LoadCursor(nullptr, id);
}
#endif

#if defined(LINUX)
X11Window* engineWindow(){
  Engine* engine = Engine::instance();
  return engine ? dynamic_cast<X11Window*>(engine->getWindow()) : nullptr;
}

::Cursor systemCursor(Display* disp, Platform::Cursor cursor){
  //created once per display
  static Display* cacheDisplay = nullptr;
  static ::Cursor cache[Platform::CURSOR_FORBIDDEN + 1] = {};
  if (disp != cacheDisplay){
    cacheDisplay = disp;
    for (::Cursor& c : cache)
      c = None;
  }
  if (cache[cursor] == None){
    unsigned shape = XC_left_ptr;
    switch (cursor){
      case Platform::CURSOR_ARROW: shape = XC_left_ptr; break;
      case Platform::CURSOR_IBEAM: shape = XC_xterm; break;
      case Platform::CURSOR_HAND: shape = XC_hand2; break;
      case Platform::CURSOR_WAIT: shape = XC_watch; break;
      case Platform::CURSOR_CROSSHAIR: shape = XC_crosshair; break;
      case Platform::CURSOR_RESIZE_H: shape = XC_sb_h_double_arrow; break;
      case Platform::CURSOR_RESIZE_V: shape = XC_sb_v_double_arrow; break;
      case Platform::CURSOR_MOVE: shape = XC_fleur; break;
      case Platform::CURSOR_FORBIDDEN: shape = XC_X_cursor; break;
    }
    cache[cursor] = XCreateFontCursor(disp, shape);
  }
  return cache[cursor];
}

//! starts a program without waiting for it and without leaving a zombie
bool spawnDetached(const char* program, const std::string& arg){
  pid_t child = fork();
  if (child < 0)
    return false;
  if (child == 0){
    //a new session, so a closed terminal doesn't take the program with it
    setsid();
    //the grandchild is adopted by init, so nobody has to wait for it; and
    //only the child is waited for, because xdg-open may stay until the
    //browser closes (seen under WSLg)
    if (fork() == 0){
      execlp(program, program, arg.c_str(), (char*)nullptr);
      _exit(127);
    }
    _exit(0);
  }
  int status = 0;
  waitpid(child, &status, 0);
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
#endif

#ifdef ANDROID
//! the activity's pointer icon (PointerIcon.TYPE_), -1 while it's the default
int shownPointerIcon = -1;

//! the calling thread's JNIEnv, attached to the VM while it exists if it
//! wasn't already
class JavaEnv{
public:
  JavaEnv(){
    if (!activity)
      return;
    if (activity->vm->GetEnv((void**)&env_, JNI_VERSION_1_6) == JNI_EDETACHED){
      if (activity->vm->AttachCurrentThread(&env_, nullptr) != JNI_OK)
        env_ = nullptr;
      else
        attached_ = true;
    }
  }
  ~JavaEnv(){
    if (attached_)
      activity->vm->DetachCurrentThread();
  }
  JNIEnv* get() const {return env_;}
private:
  JNIEnv* env_ = nullptr;
  bool attached_ = false;
};

//! a method of the activity (CgeActivity's, Engine/android/java); nullptr for
//! a plain NativeActivity
jmethodID activityMethod(JNIEnv* env, const char* name, const char* signature){
  jclass activityClass = env->GetObjectClass(activity->clazz);
  jmethodID method = env->GetMethodID(activityClass, name, signature);
  env->DeleteLocalRef(activityClass);
  if (env->ExceptionCheck()){
    //NoSuchMethodError
    env->ExceptionClear();
    return nullptr;
  }
  return method;
}

//! calls a void method of the activity with one argument
template<typename T>
void callActivity(const char* name, const char* signature, T arg){
  JavaEnv env;
  if (!env.get())
    return;
  jmethodID method = activityMethod(env.get(), name, signature);
  if (method)
    env.get()->CallVoidMethod(activity->clazz, method, arg);
  if (env.get()->ExceptionCheck())
    env.get()->ExceptionClear();
}

//! PointerIcon.TYPE_ of a cursor
int pointerIcon(Platform::Cursor cursor){
  switch (cursor){
    case Platform::CURSOR_IBEAM: return 1008; //TYPE_TEXT
    case Platform::CURSOR_HAND: return 1002; //TYPE_HAND
    case Platform::CURSOR_WAIT: return 1004; //TYPE_WAIT
    case Platform::CURSOR_CROSSHAIR: return 1007; //TYPE_CROSSHAIR
    case Platform::CURSOR_RESIZE_H: return 1014; //TYPE_HORIZONTAL_DOUBLE_ARROW
    case Platform::CURSOR_RESIZE_V: return 1015; //TYPE_VERTICAL_DOUBLE_ARROW
    case Platform::CURSOR_MOVE: return 1013; //TYPE_ALL_SCROLL
    case Platform::CURSOR_FORBIDDEN: return 1012; //TYPE_NO_DROP
    default: return 1000; //TYPE_ARROW
  }
}

//! starts an ACTION_VIEW intent for the URL through JNI
bool viewIntent(const std::string& url){
  TR_USE(CGE_Platform);
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  if (!env)
    return false;
  bool ok = false;
  jclass uriClass = env->FindClass("android/net/Uri");
  jclass intentClass = env->FindClass("android/content/Intent");
  if (uriClass && intentClass){
    jmethodID parse = env->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
    jmethodID ctor = env->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
    jmethodID addFlags = env->GetMethodID(intentClass, "addFlags", "(I)Landroid/content/Intent;");
    jclass activityClass = env->GetObjectClass(activity->clazz);
    jmethodID startActivity = env->GetMethodID(activityClass, "startActivity", "(Landroid/content/Intent;)V");
    jstring jurl = env->NewStringUTF(url.c_str());
    jstring action = env->NewStringUTF("android.intent.action.VIEW");
    jobject uri = env->CallStaticObjectMethod(uriClass, parse, jurl);
    jobject intent = env->NewObject(intentClass, ctor, action, uri);
    const jint FLAG_ACTIVITY_NEW_TASK = 0x10000000;
    jobject self = env->CallObjectMethod(intent, addFlags, FLAG_ACTIVITY_NEW_TASK);
    env->CallVoidMethod(activity->clazz, startActivity, intent);
    ok = !env->ExceptionCheck();
    env->DeleteLocalRef(self);
    env->DeleteLocalRef(intent);
    env->DeleteLocalRef(uri);
    env->DeleteLocalRef(action);
    env->DeleteLocalRef(jurl);
    env->DeleteLocalRef(activityClass);
  }
  if (env->ExceptionCheck()){
    //e.g. ActivityNotFoundException when no app handles the URL
    env->ExceptionClear();
    ok = false;
  }
  if (uriClass)
    env->DeleteLocalRef(uriClass);
  if (intentClass)
    env->DeleteLocalRef(intentClass);
  if (!ok)
    TR_WARN("cannot open %s", url.c_str());
  return ok;
}
#endif

}

uint64_t Platform::getTickMs(){
  static const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
}

bool Platform::setClipboardText(const std::string& text){
#ifdef WIN32
  //without an owner window, SetClipboardData fails after EmptyClipboard
  HWND wnd = engineWindow();
  if (!wnd || !OpenClipboard(wnd))
    return false;
  bool ok = false;
  std::wstring wide = toWide(text);
  HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (wide.size() + 1) * sizeof(wchar_t));
  if (mem){
    wchar_t* dest = (wchar_t*)GlobalLock(mem);
    memcpy(dest, wide.c_str(), (wide.size() + 1) * sizeof(wchar_t));
    GlobalUnlock(mem);
    EmptyClipboard();
    ok = SetClipboardData(CF_UNICODETEXT, mem) != nullptr;
    if (!ok)
      GlobalFree(mem);
  }
  CloseClipboard();
  return ok;
#elif defined(LINUX)
  X11Window* wnd = engineWindow();
  if (!wnd)
    return false;
  wnd->setClipboardText(text);
  return true;
#elif defined(ANDROID)
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  jmethodID method = env ? activityMethod(env, "setClipboardText", "([B)V") : nullptr;
  if (!method)
    return false;
  jbyteArray utf8 = env->NewByteArray((jsize)text.size());
  env->SetByteArrayRegion(utf8, 0, (jsize)text.size(), (const jbyte*)text.data());
  env->CallVoidMethod(activity->clazz, method, utf8);
  env->DeleteLocalRef(utf8);
  if (env->ExceptionCheck()){
    env->ExceptionClear();
    return false;
  }
  return true;
#elif defined(__EMSCRIPTEN__)
  cge_clipboard_set(text.c_str());
  return true;
#else
  return false;
#endif
}

std::string Platform::getClipboardText(){
#ifdef WIN32
  if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(engineWindow()))
    return std::string();
  std::string ret;
  HANDLE mem = GetClipboardData(CF_UNICODETEXT);
  if (mem){
    const wchar_t* wide = (const wchar_t*)GlobalLock(mem);
    if (wide)
      ret = toUtf8(wide);
    GlobalUnlock(mem);
  }
  CloseClipboard();
  return ret;
#elif defined(LINUX)
  X11Window* wnd = engineWindow();
  std::string ret;
  if (!wnd || !wnd->getClipboardText(ret))
    return std::string();
  return ret;
#elif defined(ANDROID)
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  jmethodID method = env ? activityMethod(env, "getClipboardText", "()[B") : nullptr;
  if (!method)
    return std::string();
  jbyteArray utf8 = (jbyteArray)env->CallObjectMethod(activity->clazz, method);
  std::string ret;
  if (env->ExceptionCheck())
    env->ExceptionClear();
  else if (utf8){
    ret.resize(env->GetArrayLength(utf8));
    if (!ret.empty())
      env->GetByteArrayRegion(utf8, 0, (jsize)ret.size(), (jbyte*)&ret[0]);
  }
  if (utf8)
    env->DeleteLocalRef(utf8);
  return ret;
#elif defined(__EMSCRIPTEN__)
  char* text = cge_clipboard_get();
  if (!text)
    return std::string();
  std::string ret = text;
  free(text);
  return ret;
#else
  return std::string();
#endif
}

bool Platform::hasClipboardText(){
#ifdef WIN32
  return IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
#elif defined(LINUX)
  X11Window* wnd = engineWindow();
  return wnd && wnd->hasClipboardText();
#elif defined(ANDROID)
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  jmethodID method = env ? activityMethod(env, "hasClipboardText", "()Z") : nullptr;
  if (!method)
    return false;
  bool ret = env->CallBooleanMethod(activity->clazz, method) != JNI_FALSE;
  if (env->ExceptionCheck()){
    env->ExceptionClear();
    ret = false;
  }
  return ret;
#elif defined(__EMSCRIPTEN__)
  return cge_clipboard_has_text() != 0;
#else
  return false;
#endif
}

void Platform::setCursor(Cursor cursor){
  currentCursor = cursor;
  PlatformInternal::applyCursor();
}

Platform::Cursor Platform::getCursor(){
  return currentCursor;
}

void Platform::startTextInput(){
  textInputActive = true;
#if defined(ANDROID)
  callActivity("setTextInput", "(Z)V", (jboolean)JNI_TRUE);
#elif defined(__EMSCRIPTEN__)
  EmscriptenWindow* wnd = engineWindow();
  if (wnd)
    wnd->setTextInput(true);
#endif
}

void Platform::stopTextInput(){
  if (!textInputActive)
    return;
  textInputActive = false;
#if defined(ANDROID)
  callActivity("setTextInput", "(Z)V", (jboolean)JNI_FALSE);
#elif defined(__EMSCRIPTEN__)
  EmscriptenWindow* wnd = engineWindow();
  if (wnd)
    wnd->setTextInput(false);
#endif
}

bool Platform::isTextInputActive(){
  return textInputActive;
}

bool Platform::openUrl(const std::string& url){
#if defined(WIN32)
  return (INT_PTR)ShellExecuteW(nullptr, L"open", toWide(url).c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32;
#elif defined(LINUX)
  return spawnDetached("xdg-open", url);
#elif defined(ANDROID)
  return viewIntent(url);
#elif defined(__EMSCRIPTEN__)
  return EM_ASM_INT({
    return window.open(UTF8ToString($0), "_blank") ? 1 : 0;
  }, url.c_str()) != 0;
#else
  return false;
#endif
}

bool Platform::openFile(const std::string& path){
#if defined(WIN32)
  return (INT_PTR)ShellExecuteA(nullptr, "open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32;
#elif defined(LINUX)
  return spawnDetached("xdg-open", path);
#else
  return false;
#endif
}

namespace CGE{
namespace PlatformInternal{

void init(int argc, char** argv, void* androidApp){
#if defined(WIN32)
  //the executable's name; argv[0] isn't the program name with WinMain
  char module[MAX_PATH];
  DWORD len = GetModuleFileNameA(nullptr, module, MAX_PATH);
  if (len > 0 && len < MAX_PATH){
    std::string name = module;
    name.erase(0, name.find_last_of("\\/") + 1);
    size_t dot = name.find_last_of('.');
    if (dot != std::string::npos)
      name.erase(dot);
    setDefaultAppName(name);
  }
#elif defined(ANDROID)
  android_app* app = (android_app*)androidApp;
  if (app && app->activity){
    activity = app->activity;
    shownPointerIcon = -1;
    setAssetManager(activity->assetManager);
    setAndroidDataPath(activity->internalDataPath);
  }
#elif defined(__EMSCRIPTEN__)
  cge_clipboard_install();
#else
  if (argc > 0 && argv[0]){
    std::string name = argv[0];
    name.erase(0, name.find_last_of('/') + 1);
    setDefaultAppName(name);
  }
#endif
  (void)argc;
  (void)argv;
  (void)androidApp;
}

#ifdef ANDROID
std::vector<std::string> getActivityArguments(){
  std::vector<std::string> args;
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  jmethodID method = env ? activityMethod(env, "getArguments", "()[[B") : nullptr;
  if (!method)
    return args;
  jobjectArray array = (jobjectArray)env->CallObjectMethod(activity->clazz, method);
  if (env->ExceptionCheck()){
    env->ExceptionClear();
    return args;
  }
  jsize count = array ? env->GetArrayLength(array) : 0;
  for (jsize i = 0; i < count; ++i){
    jbyteArray utf8 = (jbyteArray)env->GetObjectArrayElement(array, i);
    std::string arg((size_t)env->GetArrayLength(utf8), '\0');
    env->GetByteArrayRegion(utf8, 0, (jsize)arg.size(), (jbyte*)&arg[0]);
    env->DeleteLocalRef(utf8);
    args.push_back(arg);
  }
  if (array)
    env->DeleteLocalRef(array);
  return args;
}
#endif

void applyEngineSettings(const std::string& assetDir, const std::string& appName){
  if (!assetDir.empty())
    Assets::setRoot(assetDir);
  if (!appName.empty())
    UserDirs::setAppName(appName);
}

std::string getExecutableDir(){
  std::string path;
#if defined(WIN32)
  char module[MAX_PATH];
  DWORD len = GetModuleFileNameA(nullptr, module, MAX_PATH);
  if (len > 0 && len < MAX_PATH)
    path.assign(module, len);
#elif defined(LINUX)
  char exe[4096];
  ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe));
  if (len > 0 && len < (ssize_t)sizeof(exe))
    path.assign(exe, (size_t)len);
#endif
  size_t slash = path.find_last_of("\\/");
  if (slash == std::string::npos)
    return "";
  path.erase(slash);
  return path;
}

void setOverrideCursor(int cursor){
  if (cursor == overrideCursor)
    return;
  overrideCursor = cursor;
  applyCursor();
}

void applyCursor(){
#if defined(WIN32)
  //WM_SETCURSOR sets it again whenever the mouse moves over the window
  HWND wnd = engineWindow();
  if (!wnd)
    return;
  POINT pt;
  RECT rect;
  if (GetCursorPos(&pt) && ScreenToClient(wnd, &pt) && GetClientRect(wnd, &rect) && PtInRect(&rect, pt))
    SetCursor(systemCursor(shownCursor()));
#elif defined(LINUX)
  X11Window* wnd = engineWindow();
  Input::Mouse* mouse = Input::Mouse::instance();
  if (!wnd || !wnd->getDisplay() || (mouse && !mouse->isMouseActive()))
    return;
  XDefineCursor(wnd->getDisplay(), wnd->getWindow(), systemCursor(wnd->getDisplay(), shownCursor()));
  XFlush(wnd->getDisplay());
#elif defined(__EMSCRIPTEN__)
  EmscriptenWindow* wnd = engineWindow();
  if (!wnd)
    return;
  Input::Mouse* mouse = Input::Mouse::instance();
  cge_set_canvas_cursor(wnd->getCanvas().c_str(), mouse && !mouse->isMouseActive() ? "none" : cssCursor(shownCursor()));
#elif defined(ANDROID)
  if (!activity)
    return;
  Input::Mouse* mouse = Input::Mouse::instance();
  //TYPE_NULL hides the pointer; the UI thread sets the icon, so it's only
  //asked to when it changes
  int icon = mouse && !mouse->isMouseActive() ? 0 : pointerIcon(shownCursor());
  if (icon == shownPointerIcon)
    return;
  shownPointerIcon = icon;
  callActivity("setPointerIcon", "(I)V", (jint)icon);
#endif
}

}
}
