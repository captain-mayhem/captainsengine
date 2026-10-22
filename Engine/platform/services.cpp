#include "services.h"

#include <chrono>

#include "../io/Tracing.h"
#include "../system/engine.h"
#include "../input/mouse.h"
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
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
#endif

using namespace CGE;

TR_CHANNEL(CGE_Platform);

namespace{

Platform::Cursor currentCursor = Platform::CURSOR_ARROW;
//! the cursor ImGui wants while the mouse is over one of its windows, or -1
int overrideCursor = -1;

//! the cursor shown over the window
Platform::Cursor shownCursor(){
  return overrideCursor >= 0 ? (Platform::Cursor)overrideCursor : currentCursor;
}
#ifdef ANDROID
ANativeActivity* activity = nullptr;
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
    //the grandchild is adopted by init, so nobody has to wait for it
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
//! starts an ACTION_VIEW intent for the URL through JNI
bool viewIntent(const std::string& url){
  TR_USE(CGE_Platform);
  if (!activity)
    return false;
  JNIEnv* env = nullptr;
  bool attached = false;
  if (activity->vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK){
    if (activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
      return false;
    attached = true;
  }
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
  if (attached)
    activity->vm->DetachCurrentThread();
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
#else
  return std::string();
#endif
}

bool Platform::hasClipboardText(){
#ifdef WIN32
  return IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
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
    setAssetManager(activity->assetManager);
    setAndroidDataPath(activity->internalDataPath);
  }
#elif !defined(__EMSCRIPTEN__)
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

void applyEngineSettings(const std::string& assetDir, const std::string& appName){
  if (!assetDir.empty())
    Assets::setRoot(assetDir);
  if (!appName.empty())
    UserDirs::setAppName(appName);
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
#endif
}

}
}
