#include "services.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>

#include "../io/Tracing.h"
#include "../system/engine.h"
#include "../system/file.h"
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
#include <commdlg.h>
#include <wininet.h>
#endif
#if defined(LINUX)
#include <X11/cursorfont.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#ifdef ANDROID
#include <android/configuration.h>
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

//! the browser's languages, separated by commas
EM_JS(char*, cge_languages, (), {
  if (typeof navigator === "undefined")
    return 0;
  var list = navigator.languages && navigator.languages.length ? navigator.languages : [navigator.language];
  var s = list.filter(function(l){ return !!l; }).join(",");
  return s ? stringToNewUTF8(s) : 0;
});

//! sets the CSS cursor of the canvas
// Importing and exporting files (guit3's websaves, made general). An import
// opens a hidden <input type="file">; the chosen file is written into the
// folder (a free name as Platform::importedName gives, or replacing one of
// the same name through a temporary file), committed to IndexedDB, and then
// its outcome is left in Module.cgeTransfer for Platform::pollTransfer:
// status 1 running, 2 done (name: the file's name in the folder), 3
// cancelled, 4 failed (Platform::TransferStatus). An export is a download of
// a Blob under the file's name. A browser opens the picker only shortly
// after a click, a tap or a key (transient user activation); without it the
// import doesn't start. A picker still open gives way to a new transfer: a
// browser without the input's cancel event never tells that it was closed.
EM_JS_DEPS(cge_transfer, "$UTF8ToString,$FS");

// Module.cgeStoreFile(dir, name, buf, replace, fallback): writes an imported
// or downloaded file into dir under the cleaned name (fallback if nothing is
// left of it), as Platform::importedName names it, and returns that name.
// Without replace a taken name gets " (2)" etc.; with it the file is written
// under a temporary name first and renamed, so a file still open keeps its
// old node.
EM_JS(void, cge_transfer_install, (), {
  if (Module.cgeStoreFile)
    return;
  Module.cgeStoreFile = function(dir, rawName, buf, replace, fallback) {
    // no backslash literals: EM_JS stringifies this code
    var name = "";
    for (var i = 0; i < rawName.length; ++i) {
      var c = rawName.charCodeAt(i);
      name += (c === 47 || c === 92 || c < 32 || c === 127) ? '_' : rawName.charAt(i);
    }
    if (name === "" || name === '.' || name === '..')
      name = fallback;
    FS.mkdirTree(dir);
    if (replace) {
      var tmp = dir + '/.transfer.tmp';
      FS.writeFile(tmp, new Uint8Array(buf));
      FS.rename(tmp, dir + '/' + name);
    } else {
      var dot = name.lastIndexOf('.');
      var stem = dot > 0 ? name.substring(0, dot) : name;
      var ext = dot > 0 ? name.substring(dot) : "";
      for (var n = 2; FS.analyzePath(dir + '/' + name).exists; ++n)
        name = stem + ' (' + n + ')' + ext;
      FS.writeFile(dir + '/' + name, new Uint8Array(buf));
    }
    return name;
  };
});

EM_JS(int, cge_transfer_import, (const char* folder_c, const char* accept_c, int replace), {
  var dir = UTF8ToString(folder_c);
  if (navigator.userActivation && !navigator.userActivation.isActive)
    return 0;
  if (Module.cgeTransfer && Module.cgeTransfer.status === 1 && !Module.cgeTransfer.choosing)
    return 0;
  var state = { status: 1, name: "", choosing: true };
  Module.cgeTransfer = state;

  var input = document.createElement('input');
  input.type = 'file';
  input.accept = UTF8ToString(accept_c);
  input.style.display = 'none';
  document.body.appendChild(input);
  var finish = function(status, name) {
    if (state.status === 1 && Module.cgeTransfer === state) {
      state.status = status;
      state.name = name || "";
    }
    if (input.parentNode)
      input.parentNode.removeChild(input);
  };
  input.addEventListener('cancel', function() { finish(3); });
  input.addEventListener('change', function() {
    var file = input.files && input.files[0];
    if (!file || Module.cgeTransfer !== state) {
      finish(3);
      return;
    }
    state.choosing = false;
    file.arrayBuffer().then(function(buf) {
      var name = Module.cgeStoreFile(dir, file.name, buf, replace, 'upload');
      FS.syncfs(false, function(err) {
        if (err)
          console.error('cge: committing the import to IndexedDB failed', err);
        finish(2, name);
      });
    }).catch(function(err) {
      console.error('cge: importing a file failed', err);
      finish(4);
    });
  });
  input.click();
  return 1;
});

// the transfer's status, and for a done one its name (UTF-8) in buf; an
// ended transfer is told once
EM_JS(int, cge_transfer_poll, (char* buf, int buflen), {
  var state = Module.cgeTransfer;
  if (!state)
    return 0;
  if (state.status === 2 && buflen > 0) {
    var bytes = new TextEncoder().encode(state.name);
    var n = Math.min(bytes.length, buflen - 1);
    HEAPU8.set(bytes.subarray(0, n), buf);
    HEAPU8[buf + n] = 0;
  }
  var status = state.status;
  if (status !== 1)
    Module.cgeTransfer = null;
  return status;
});

EM_JS(int, cge_transfer_download, (const char* path_c), {
  var path = UTF8ToString(path_c);
  if (Module.cgeTransfer && Module.cgeTransfer.status === 1 && !Module.cgeTransfer.choosing)
    return 0;
  try {
    var data = FS.readFile(path);
    var blob = new Blob([data], { type: 'application/octet-stream' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = path.substring(path.lastIndexOf('/') + 1);
    a.style.display = 'none';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    setTimeout(function() { URL.revokeObjectURL(url); }, 10000);
    Module.cgeTransfer = { status: 2, name: a.download };
    return 1;
  } catch (err) {
    console.error('cge: exporting ' + path + ' failed', err);
    Module.cgeTransfer = { status: 4, name: "" };
    return 1;
  }
});

// a download with fetch(), stored like an import; the name is the last path
// segment of the URL the response came from (after redirections)
EM_JS(int, cge_transfer_fetch, (const char* url_c, const char* folder_c, int replace), {
  var url = UTF8ToString(url_c);
  var dir = UTF8ToString(folder_c);
  if (Module.cgeTransfer && Module.cgeTransfer.status === 1 && !Module.cgeTransfer.choosing)
    return 0;
  var state = { status: 1, name: "", choosing: false };
  Module.cgeTransfer = state;
  var finish = function(status, name) {
    if (state.status === 1 && Module.cgeTransfer === state) {
      state.status = status;
      state.name = name || "";
    }
  };
  fetch(url).then(function(response) {
    if (!response.ok)
      throw new Error('HTTP status ' + response.status);
    var path = new URL(response.url || url).pathname;
    var segment = path.substring(path.lastIndexOf('/') + 1);
    try {
      segment = decodeURIComponent(segment);
    } catch (e) {
      // keep it as it is
    }
    return response.arrayBuffer().then(function(buf) {
      if (Module.cgeTransfer !== state)
        return;
      var name = Module.cgeStoreFile(dir, segment, buf, replace, 'download');
      FS.syncfs(false, function(err) {
        if (err)
          console.error('cge: committing the download to IndexedDB failed', err);
        finish(2, name);
      });
    });
  }).catch(function(err) {
    console.error('cge: downloading ' + url + ' failed', err);
    finish(4);
  });
  return 1;
});

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

// --- importing and exporting files ---

namespace{

#if defined(WIN32) || defined(LINUX)
//! the transfer's state: the dialogs are modal, so theirs is known at once;
//! a download runs on a thread of its own, which sets it when it ends
std::mutex transferMutex;
Platform::TransferStatus transferStatus = Platform::TRANSFER_NONE;
std::string transferName;

//! marks a transfer as running; false if one already runs
bool beginTransfer(){
  std::lock_guard<std::mutex> lock(transferMutex);
  if (transferStatus == Platform::TRANSFER_RUNNING)
    return false;
  transferStatus = Platform::TRANSFER_RUNNING;
  transferName.clear();
  return true;
}

void finishTransfer(Platform::TransferStatus status, const std::string& name = std::string()){
  std::lock_guard<std::mutex> lock(transferMutex);
  transferStatus = status;
  transferName = status == Platform::TRANSFER_DONE ? name : std::string();
}

//! the last path segment of a URL, percent-decoded, without its query and
//! fragment; empty if the URL has no path
std::string urlFileName(const std::string& url){
  std::string path = url.substr(0, url.find_first_of("?#"));
  const size_t scheme = path.find("://");
  const size_t start = scheme == std::string::npos ? 0 : scheme + 3;
  const size_t firstSlash = path.find('/', start);
  if (firstSlash == std::string::npos)
    return std::string();
  const std::string segment = path.substr(path.rfind('/') + 1);
  std::string name;
  for (size_t i = 0; i < segment.size(); ++i){
    if (segment[i] == '%' && i + 2 < segment.size() && isxdigit((unsigned char)segment[i + 1]) &&
        isxdigit((unsigned char)segment[i + 2])){
      name += (char)strtol(segment.substr(i + 1, 2).c_str(), nullptr, 16);
      i += 2;
    }
    else
      name += segment[i];
  }
  return name;
}

//! moves a downloaded temporary file to its name in folder; the name
//! (from importedName) or empty if that failed
std::string storeDownload(const std::string& temporary, const std::string& folder, const std::string& rawName,
                          bool replace){
  const std::string name = Platform::importedName(folder, rawName.empty() ? "download" : rawName, replace);
  const std::string path = folder + "/" + name;
#ifdef WIN32
  const bool moved = MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
#else
  const bool moved = std::rename(temporary.c_str(), path.c_str()) == 0;
#endif
  if (!moved){
    std::remove(temporary.c_str());
    return std::string();
  }
  return name;
}
#endif

#ifdef WIN32
//! a UTF-8 text in the ANSI code page (the engine's narrow file names)
std::string toAnsi(const std::string& utf8){
  const std::wstring wide = toWide(utf8);
  if (wide.empty())
    return std::string();
  const int size = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
  std::string ansi(size > 0 ? size : 0, '\0');
  if (size > 0)
    WideCharToMultiByte(CP_ACP, 0, wide.c_str(), (int)wide.size(), &ansi[0], size, nullptr, nullptr);
  return ansi;
}

//! downloads url into folder with WinINet, which follows redirections and
//! uses the system's proxy and certificates
void downloadThread(std::string url, std::string folder, bool replace){
  TR_USE(CGE_Platform);
  HINTERNET net = InternetOpenW(L"CGE", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
  HINTERNET request = net ? InternetOpenUrlW(net, toWide(url).c_str(), nullptr, 0,
                                             INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI,
                                             0)
                          : nullptr;
  DWORD code = 0;
  DWORD size = sizeof(code);
  bool ok = request && HttpQueryInfoW(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &code, &size, nullptr) &&
            code >= 200 && code < 300;
  if (request && !ok)
    TR_WARN("downloading %s: HTTP status %u", url.c_str(), (unsigned)code);
  else if (!request)
    TR_WARN("downloading %s: can't connect (%u)", url.c_str(), (unsigned)GetLastError());

  //the URL after redirections names the file
  std::string finalUrl = url;
  if (ok){
    DWORD length = 0;
    InternetQueryOptionW(request, INTERNET_OPTION_URL, nullptr, &length);
    if (length > 0){
      std::wstring buffer(length / sizeof(wchar_t) + 1, L'\0');
      if (InternetQueryOptionW(request, INTERNET_OPTION_URL, &buffer[0], &length))
        finalUrl = toUtf8(buffer.c_str());
    }
  }

  const std::string temporary = folder + "/.download.tmp";
  if (ok){
    Filesystem::createDirs(folder);
    FILE* out = std::fopen(temporary.c_str(), "wb");
    ok = out != nullptr;
    char buffer[65536];
    DWORD read = 0;
    while (ok){
      if (!InternetReadFile(request, buffer, sizeof(buffer), &read))
        ok = false;
      else if (read == 0)
        break;
      else if (std::fwrite(buffer, 1, read, out) != read)
        ok = false;
    }
    if (out && std::fclose(out) != 0)
      ok = false;
    if (!ok)
      std::remove(temporary.c_str());
  }
  if (request)
    InternetCloseHandle(request);
  if (net)
    InternetCloseHandle(net);

  const std::string name = ok ? storeDownload(temporary, folder, toAnsi(urlFileName(finalUrl)), replace) : "";
  finishTransfer(name.empty() ? Platform::TRANSFER_FAILED : Platform::TRANSFER_DONE, name);
}

//! the dialogs' filter for the accepted extensions (".zip,.txt"), and all
//! files: "Files (*.zip;*.txt)\0*.zip;*.txt\0All files (*.*)\0*.*\0\0"
std::string dialogFilter(const std::string& accept){
  std::string patterns;
  size_t start = 0;
  while (start < accept.size()){
    size_t end = accept.find(',', start);
    if (end == std::string::npos)
      end = accept.size();
    std::string ext = accept.substr(start, end - start);
    ext.erase(0, ext.find_first_not_of(" ."));
    ext.erase(ext.find_last_not_of(' ') + 1);
    if (!ext.empty())
      patterns += (patterns.empty() ? "*." : ";*.") + ext;
    start = end + 1;
  }
  std::string filter;
  if (!patterns.empty())
    filter += "Files (" + patterns + ")" + '\0' + patterns + '\0';
  filter += std::string("All files (*.*)") + '\0' + "*.*" + '\0' + '\0';
  return filter;
}

//! the window that the dialogs belong to
HWND dialogOwner(){
  return GetActiveWindow();
}
#endif

#ifdef LINUX
//! runs a program and waits for it; its standard output in output; the
//! exit status, 127 if it couldn't be run
int runProgram(const std::vector<std::string>& args, std::string& output){
  std::vector<char*> argv;
  for (const std::string& arg : args)
    argv.push_back(const_cast<char*>(arg.c_str()));
  argv.push_back(nullptr);
  int pipeFds[2];
  if (pipe(pipeFds) != 0)
    return 127;
  const pid_t pid = fork();
  if (pid < 0){
    close(pipeFds[0]);
    close(pipeFds[1]);
    return 127;
  }
  if (pid == 0){
    dup2(pipeFds[1], STDOUT_FILENO);
    close(pipeFds[0]);
    close(pipeFds[1]);
    execvp(argv[0], argv.data());
    _exit(127);
  }
  close(pipeFds[1]);
  char buffer[1024];
  ssize_t n;
  while ((n = read(pipeFds[0], buffer, sizeof(buffer))) > 0)
    output.append(buffer, (size_t)n);
  close(pipeFds[0]);
  int status = 0;
  if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status))
    return 127;
  return WEXITSTATUS(status);
}

//! downloads url into folder with the curl program (or wget), which follow
//! redirections; curl tells the URL it ended at
void downloadThread(std::string url, std::string folder, bool replace){
  TR_USE(CGE_Platform);
  Filesystem::createDirs(folder);
  const std::string temporary = folder + "/.download.tmp";
  std::string finalUrl;
  int status = runProgram({"curl", "-fsSL", "--proto", "=http,https", "-o", temporary, "-w", "%{url_effective}", url},
                          finalUrl);
  if (status == 127){
    finalUrl.clear();
    status = runProgram({"wget", "-q", "-O", temporary, url}, finalUrl);
    finalUrl.clear();
  }
  if (status != 0){
    TR_WARN("downloading %s failed (exit status %d)", url.c_str(), status);
    std::remove(temporary.c_str());
    finishTransfer(Platform::TRANSFER_FAILED);
    return;
  }
  const std::string name = storeDownload(temporary, folder, urlFileName(finalUrl.empty() ? url : finalUrl), replace);
  finishTransfer(name.empty() ? Platform::TRANSFER_FAILED : Platform::TRANSFER_DONE, name);
}
#endif

#ifdef ANDROID
//! calls a boolean method of the activity with byte array arguments (UTF-8
//! strings) and a boolean; false without the method
bool callActivityBool(const char* name, const char* signature, const std::vector<std::string>& texts, bool flag,
                      bool withFlag){
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  jmethodID method = env ? activityMethod(env, name, signature) : nullptr;
  if (!method)
    return false;
  std::vector<jvalue> args;
  for (const std::string& text : texts){
    jbyteArray utf8 = env->NewByteArray((jsize)text.size());
    env->SetByteArrayRegion(utf8, 0, (jsize)text.size(), (const jbyte*)text.data());
    jvalue arg;
    arg.l = utf8;
    args.push_back(arg);
  }
  if (withFlag){
    jvalue arg;
    arg.z = flag ? JNI_TRUE : JNI_FALSE;
    args.push_back(arg);
  }
  const bool result = env->CallBooleanMethodA(activity->clazz, method, args.data()) == JNI_TRUE;
  for (size_t i = 0; i < texts.size(); ++i)
    env->DeleteLocalRef(args[i].l);
  if (env->ExceptionCheck()){
    env->ExceptionClear();
    return false;
  }
  return result;
}
#endif

}

bool Platform::canTransferFiles(){
#if defined(WIN32) || defined(__EMSCRIPTEN__)
  return true;
#elif defined(ANDROID)
  JavaEnv javaEnv;
  return javaEnv.get() && activityMethod(javaEnv.get(), "importFile", "([B[BZ)Z") != nullptr;
#else
  return false;
#endif
}

std::string Platform::importedName(const std::string& folder, const std::string& name, bool replace){
  std::string clean;
  for (char ch : name){
    const unsigned char c = (unsigned char)ch;
    clean += (c == '/' || c == '\\' || c < 0x20 || c == 0x7f) ? '_' : ch;
  }
  if (clean.empty() || clean == "." || clean == "..")
    clean = "upload";
  if (replace)
    return clean;
  const size_t dot = clean.rfind('.');
  const std::string stem = dot != std::string::npos && dot > 0 ? clean.substr(0, dot) : clean;
  const std::string ext = dot != std::string::npos && dot > 0 ? clean.substr(dot) : "";
  std::string result = clean;
  for (int n = 2; Filesystem::exists(folder + "/" + result); ++n)
    result = stem + " (" + std::to_string(n) + ")" + ext;
  return result;
}

bool Platform::importFile(const std::string& folder, const std::string& accept, bool replace){
#if defined(WIN32)
  if (!beginTransfer())
    return false;
  const std::string filter = dialogFilter(accept);
  char chosen[MAX_PATH] = "";
  OPENFILENAMEA ofn = {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = dialogOwner();
  ofn.lpstrFilter = filter.c_str();
  ofn.lpstrFile = chosen;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  if (!GetOpenFileNameA(&ofn)){
    finishTransfer(CommDlgExtendedError() == 0 ? TRANSFER_CANCELLED : TRANSFER_FAILED);
    return true;
  }
  Filesystem::createDirs(folder);
  const std::string name = importedName(folder, Filesystem::getFileComponent(chosen), replace);
  const bool copied = CopyFileA(chosen, (folder + "/" + name).c_str(), FALSE) != 0;
  finishTransfer(copied ? TRANSFER_DONE : TRANSFER_FAILED, name);
  return true;
#elif defined(ANDROID)
  return callActivityBool("importFile", "([B[BZ)Z", {folder, accept}, replace, true);
#elif defined(__EMSCRIPTEN__)
  return cge_transfer_import(folder.c_str(), accept.c_str(), replace ? 1 : 0) != 0;
#else
  return false;
#endif
}

bool Platform::exportFile(const std::string& path){
#if defined(WIN32)
  if (!Filesystem::exists(path) || !beginTransfer())
    return false;
  const std::string name = Filesystem::getFileComponent(path);
  const std::string ext = Filesystem::getExtension(path);
  const std::string filter = dialogFilter(ext.empty() ? "" : "." + ext);
  char chosen[MAX_PATH] = "";
  std::snprintf(chosen, sizeof(chosen), "%s", name.c_str());
  OPENFILENAMEA ofn = {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = dialogOwner();
  ofn.lpstrFilter = filter.c_str();
  ofn.lpstrFile = chosen;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = ext.empty() ? nullptr : ext.c_str();
  ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  if (!GetSaveFileNameA(&ofn)){
    finishTransfer(CommDlgExtendedError() == 0 ? TRANSFER_CANCELLED : TRANSFER_FAILED);
    return true;
  }
  const bool copied = CopyFileA(path.c_str(), chosen, FALSE) != 0;
  finishTransfer(copied ? TRANSFER_DONE : TRANSFER_FAILED, Filesystem::getFileComponent(chosen));
  return true;
#elif defined(ANDROID)
  return callActivityBool("exportFile", "([B)Z", {path}, false, false);
#elif defined(__EMSCRIPTEN__)
  return cge_transfer_download(path.c_str()) != 0;
#else
  return false;
#endif
}

bool Platform::canDownloadFiles(){
#if defined(WIN32) || defined(LINUX) || defined(__EMSCRIPTEN__)
  return true;
#elif defined(ANDROID)
  JavaEnv javaEnv;
  return javaEnv.get() && activityMethod(javaEnv.get(), "downloadFile", "([B[BZ)Z") != nullptr;
#else
  return false;
#endif
}

bool Platform::downloadFile(const std::string& url, const std::string& folder, bool replace){
  std::string scheme = url.substr(0, url.find("://"));
  std::transform(scheme.begin(), scheme.end(), scheme.begin(), [](unsigned char c){return (char)tolower(c);});
  if ((scheme != "http" && scheme != "https") || url.find("://") == std::string::npos)
    return false;
#if defined(WIN32) || defined(LINUX)
  if (!beginTransfer())
    return false;
  std::thread(downloadThread, url, folder, replace).detach();
  return true;
#elif defined(ANDROID)
  return callActivityBool("downloadFile", "([B[BZ)Z", {url, folder}, replace, true);
#elif defined(__EMSCRIPTEN__)
  return cge_transfer_fetch(url.c_str(), folder.c_str(), replace ? 1 : 0) != 0;
#else
  (void)folder;
  (void)replace;
  return false;
#endif
}

Platform::TransferStatus Platform::pollTransfer(std::string* name){
#if defined(WIN32) || defined(LINUX)
  std::lock_guard<std::mutex> lock(transferMutex);
  const TransferStatus status = transferStatus;
  if (name)
    *name = status == TRANSFER_DONE ? transferName : "";
  if (status != TRANSFER_RUNNING)
    transferStatus = TRANSFER_NONE;
  return status;
#elif defined(ANDROID)
  // the status in the first byte, then the name (UTF-8)
  JavaEnv javaEnv;
  JNIEnv* env = javaEnv.get();
  jmethodID method = env ? activityMethod(env, "pollTransfer", "()[B") : nullptr;
  if (!method)
    return TRANSFER_NONE;
  jbyteArray result = (jbyteArray)env->CallObjectMethod(activity->clazz, method);
  if (env->ExceptionCheck() || !result){
    env->ExceptionClear();
    return TRANSFER_NONE;
  }
  const jsize length = env->GetArrayLength(result);
  std::string bytes(length, '\0');
  if (length > 0)
    env->GetByteArrayRegion(result, 0, length, (jbyte*)&bytes[0]);
  env->DeleteLocalRef(result);
  if (bytes.empty())
    return TRANSFER_NONE;
  if (name)
    *name = bytes.substr(1);
  return (TransferStatus)bytes[0];
#elif defined(__EMSCRIPTEN__)
  char buffer[512] = "";
  const int status = cge_transfer_poll(buffer, sizeof(buffer));
  if (name)
    *name = status == TRANSFER_DONE ? buffer : "";
  return (TransferStatus)status;
#else
  if (name)
    name->clear();
  return TRANSFER_NONE;
#endif
}

namespace{

//! a language as a BCP 47 tag: POSIX's "de_DE.UTF-8@euro" becomes "de-DE";
//! empty for C and POSIX
std::string languageTag(std::string name){
  name = name.substr(0, name.find_first_of(".@"));
  if (name == "C" || name == "POSIX")
    return std::string();
  for (char& c : name){
    if (c == '_')
      c = '-';
  }
  return name;
}

//! the tags of a list separated by the character, empty ones left out
void addLanguages(std::vector<std::string>& languages, const std::string& list, char separator){
  size_t start = 0;
  while (start <= list.size()){
    size_t end = list.find(separator, start);
    if (end == std::string::npos)
      end = list.size();
    std::string tag = languageTag(list.substr(start, end - start));
    if (!tag.empty() && std::find(languages.begin(), languages.end(), tag) == languages.end())
      languages.push_back(tag);
    start = end + 1;
  }
}

}

std::vector<std::string> Platform::getLanguages(){
  std::vector<std::string> languages;
#if defined(WIN32)
  ULONG count = 0;
  ULONG size = 0;
  if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &size) && size > 0){
    //names separated by zeros, ending with two
    std::vector<wchar_t> names(size);
    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, names.data(), &size)){
      for (const wchar_t* name = names.data(); *name; name += wcslen(name) + 1)
        addLanguages(languages, toUtf8(name), ',');
    }
  }
#elif defined(LINUX)
  //as gettext: the locale of messages, and LANGUAGE's list before it unless that is C
  std::string locale;
  for (const char* var : {"LC_ALL", "LC_MESSAGES", "LANG"}){
    const char* value = getenv(var);
    if (value && *value){
      locale = value;
      break;
    }
  }
  if (!languageTag(locale).empty()){
    const char* list = getenv("LANGUAGE");
    if (list)
      addLanguages(languages, list, ':');
    addLanguages(languages, locale, ':');
  }
#elif defined(ANDROID)
  if (activity && activity->assetManager){
    AConfiguration* config = AConfiguration_new();
    AConfiguration_fromAssetManager(config, activity->assetManager);
    char language[2] = {};
    char country[2] = {};
    AConfiguration_getLanguage(config, language);
    AConfiguration_getCountry(config, country);
    AConfiguration_delete(config);
    if (language[0]){
      std::string tag(language, language[1] ? 2 : 1);
      if (country[0])
        tag += "-" + std::string(country, country[1] ? 2 : 1);
      languages.push_back(tag);
    }
  }
#elif defined(__EMSCRIPTEN__)
  char* list = cge_languages();
  if (list){
    addLanguages(languages, list, ',');
    free(list);
  }
#endif
  return languages;
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
  cge_transfer_install();
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
