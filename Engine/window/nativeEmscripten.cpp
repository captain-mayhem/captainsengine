#if defined(__EMSCRIPTEN__)
#include <algorithm>
#include <cmath>

#include <emscripten.h>
#include <emscripten/html5.h>

#include "../system/engine.h"
#include "../system/script.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../input/touch.h"
#include "../io/TraceManager.h"
#include "../ui/imgui_impl_cge_platform.h"
#include "nativeEmscripten.h"

TR_CHANNEL_EXT(CGE_Window);

using namespace CGE;

namespace{

//! the window that gets the events
EmscriptenWindow* eventWindow = nullptr;
//! the buttons held down (MB_ bits)
int buttonState = 0;
//! the mouse is outside the canvas (ImGui was told so)
bool mouseOutside = true;

//! the window, while the engine runs with it
EmscriptenWindow* activeWindow(){
  Engine* engine = Engine::instance();
  return engine && engine->getWindow() == eventWindow ? eventWindow : nullptr;
}

int toButton(unsigned short button){
  switch (button){
    case 0: return MB_LEFT;
    case 1: return MB_MIDDLE;
    case 2: return MB_RIGHT;
    default: return 0;
  }
}

int toImGuiButton(int button){
  switch (button){
    case MB_LEFT: return ImGuiMouseButton_Left;
    case MB_RIGHT: return ImGuiMouseButton_Right;
    default: return ImGuiMouseButton_Middle;
  }
}

//! the buttons and the modifier bits for Input::Mouse::move
int mouseState(bool shift, bool ctrl){
  return buttonState | (shift ? MB_SHIFT : 0) | (ctrl ? MB_CTRL : 0);
}

void press(int x, int y, int button){
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  ImGui_ImplCGE_Platform_AddMouseButton(toImGuiButton(button), true);
  buttonState |= button;
  mouseOutside = false;
  Input::Mouse::instance()->press(x, y, button);
}

void release(int x, int y, int button){
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  ImGui_ImplCGE_Platform_AddMouseButton(toImGuiButton(button), false);
  buttonState &= ~button;
  Input::Mouse::instance()->buttonUp(x, y, button);
}

void move(int x, int y, int state){
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  Input::Mouse::instance()->move(x, y, state);
}

void modifiers(bool ctrl, bool shift, bool alt, bool meta){
  ImGui_ImplCGE_Platform_AddModifiers(ctrl, shift, alt, meta);
}

// --- mouse ---

//! on the whole page, so a drag that leaves the canvas still ends
EM_BOOL onMouseDown(int, const EmscriptenMouseEvent* event, void*){
  EmscriptenWindow* wnd = activeWindow();
  int button = toButton(event->button);
  if (!wnd || !button)
    return EM_FALSE;
  int x, y;
  wnd->toWindow(event->clientX, event->clientY, x, y);
  //the page's other elements keep their clicks
  if (!wnd->contains(x, y))
    return EM_FALSE;
  modifiers(event->ctrlKey, event->shiftKey, event->altKey, event->metaKey);
  press(x, y, button);
  return EM_TRUE;
}

EM_BOOL onMouseUp(int, const EmscriptenMouseEvent* event, void*){
  EmscriptenWindow* wnd = activeWindow();
  int button = toButton(event->button);
  if (!wnd || !(buttonState & button))
    return EM_FALSE;
  int x, y;
  wnd->toWindow(event->clientX, event->clientY, x, y);
  modifiers(event->ctrlKey, event->shiftKey, event->altKey, event->metaKey);
  release(x, y, button);
  return wnd->contains(x, y);
}

EM_BOOL onMouseMove(int, const EmscriptenMouseEvent* event, void*){
  EmscriptenWindow* wnd = activeWindow();
  if (!wnd)
    return EM_FALSE;
  int x, y;
  wnd->toWindow(event->clientX, event->clientY, x, y);
  if (!wnd->contains(x, y) && buttonState == 0){
    if (!mouseOutside)
      ImGui_ImplCGE_Platform_AddMouseLeave();
    mouseOutside = true;
    return EM_FALSE;
  }
  mouseOutside = false;
  move(x, y, mouseState(event->shiftKey, event->ctrlKey));
  return EM_FALSE;
}

EM_BOOL onWheel(int, const EmscriptenWheelEvent* event, void*){
  EmscriptenWindow* wnd = activeWindow();
  if (!wnd)
    return EM_FALSE;
  int x, y;
  wnd->toWindow(event->mouse.clientX, event->mouse.clientY, x, y);
  //a notch is about 100 pixels or 3 lines
  double perNotch = event->deltaMode == DOM_DELTA_PIXEL ? 100 : (event->deltaMode == DOM_DELTA_LINE ? 3 : 1);
  ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
  ImGui_ImplCGE_Platform_AddMouseWheel((float)(-event->deltaX/perNotch), (float)(-event->deltaY/perNotch));
  if (event->deltaY != 0)
    Input::Mouse::instance()->wheel(x, y, event->deltaY < 0 ? 1 : -1);
  //the page doesn't scroll
  return EM_TRUE;
}

// --- touch ---

//! the fingers go to Input::Touch (which makes the first one ImGui's mouse,
//! and the application's left mouse button with its mouse emulation)
EM_BOOL onTouch(int type, const EmscriptenTouchEvent* event, void*){
  EmscriptenWindow* wnd = activeWindow();
  Input::Touch* touch = Input::Touch::instance();
  if (!wnd || !touch)
    return EM_FALSE;
  for (int i = 0; i < event->numTouches; ++i){
    const EmscriptenTouchPoint& point = event->touches[i];
    if (!point.isChanged)
      continue;
    //the browser keeps an identifier for a touch until it ends
    int id = (int)point.identifier;
    int x, y;
    wnd->toWindow(point.clientX, point.clientY, x, y);
    switch (type){
      case EMSCRIPTEN_EVENT_TOUCHSTART:
        touch->down(id, x, y);
        break;
      case EMSCRIPTEN_EVENT_TOUCHMOVE:
        touch->move(id, x, y);
        break;
      case EMSCRIPTEN_EVENT_TOUCHEND:
        touch->up(id, x, y);
        break;
      default:
        touch->cancel(id);
        break;
    }
  }
  //no scrolling or zooming, and no mouse events made from the touches
  return EM_TRUE;
}

// --- keyboard ---

//! keys whose browser function stays: copy, cut and paste (the paste event
//! brings the clipboard's text), reload, full screen and the developer tools
bool isBrowserKey(const EmscriptenKeyboardEvent* event){
  if (event->ctrlKey || event->metaKey){
    unsigned long key = event->keyCode;
    if (key == 'C' || key == 'V' || key == 'X')
      return true;
  }
  return event->keyCode == 116 || event->keyCode == 122 || event->keyCode == 123;
}

//! the character a key event types: its key is a single code point (not a
//! name like "Enter" or "Dead"), without Ctrl (AltGr is Ctrl+Alt) or Meta
bool typedChar(const EmscriptenKeyboardEvent* event, unsigned& ch){
  if ((event->ctrlKey && !event->altKey) || event->metaKey)
    return false;
  const unsigned char* s = (const unsigned char*)event->key;
  int len;
  if (s[0] < 0x80){
    ch = s[0];
    len = 1;
  }
  else if ((s[0] & 0xE0) == 0xC0){
    ch = s[0] & 0x1F;
    len = 2;
  }
  else if ((s[0] & 0xF0) == 0xE0){
    ch = s[0] & 0x0F;
    len = 3;
  }
  else if ((s[0] & 0xF8) == 0xF0){
    ch = s[0] & 0x07;
    len = 4;
  }
  else
    return false;
  for (int i = 1; i < len; ++i){
    if ((s[i] & 0xC0) != 0x80)
      return false;
    ch = (ch << 6) | (s[i] & 0x3F);
  }
  return s[0] != 0 && s[len] == 0;
}

EM_BOOL onKeyDown(int, const EmscriptenKeyboardEvent* event, void*){
  if (!activeWindow())
    return EM_FALSE;
  modifiers(event->ctrlKey, event->shiftKey, event->altKey, event->metaKey);
  int key = (int)event->keyCode;
  if (key != 0){
    ImGui_ImplCGE_Platform_AddKey(key, true);
    Input::Keyboard::instance()->keyDown(key);
  }
  //the character comes with the key down; preventing its default also
  //prevents the keypress event
  unsigned ch;
  if (typedChar(event, ch))
    Input::Keyboard::instance()->character(ch);
  return !isBrowserKey(event);
}

EM_BOOL onKeyUp(int, const EmscriptenKeyboardEvent* event, void*){
  if (!activeWindow())
    return EM_FALSE;
  modifiers(event->ctrlKey, event->shiftKey, event->altKey, event->metaKey);
  int key = (int)event->keyCode;
  if (key != 0){
    ImGui_ImplCGE_Platform_AddKey(key, false);
    Input::Keyboard::instance()->keyUp(key);
  }
  return !isBrowserKey(event);
}

// --- the page ---

EM_BOOL onFocus(int type, const EmscriptenFocusEvent*, void*){
  if (activeWindow())
    ImGui_ImplCGE_Platform_AddFocus(type == EMSCRIPTEN_EVENT_FOCUS);
  return EM_FALSE;
}

//! a hidden tab gets no frames, so it's paused
EM_BOOL onVisibility(int, const EmscriptenVisibilityChangeEvent* event, void*){
  if (!activeWindow())
    return EM_FALSE;
  if (event->hidden)
    Engine::instance()->pause();
  else
    Engine::instance()->resume();
  return EM_FALSE;
}

EM_BOOL onContextLost(int, const void*, void*){
  TR_USE(CGE_Window);
  TR_ERROR("The WebGL context was lost; the page has to be reloaded");
  return EM_FALSE;
}

//! the engine's tracing in the browser's console
class BrowserConsoleOutputter : public TraceOutputter{
public:
  bool init() override {return true;}
  void trace(unsigned channel, int level, const char* function, const char* message) override{
    EM_ASM({
      var text = $1 + "-" + UTF8ToString($2) + ": " + UTF8ToString($3);
      if ($0 <= 2)
        console.error(text);
      else if ($0 == 3)
        console.warn(text);
      else
        console.log(text);
    }, level <= TRACE_ERROR ? 2 : level, level, function, message);
  }
};

}

EmscriptenWindow::EmscriptenWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc){
  left_ = 0;
  top_ = 0;
  scale_ = 1;
  fixedSize_ = false;
  fullscreen_ = false;
}

void EmscriptenWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  canvas_ = Script::instance()->getStringSetting("canvas", false);
  if (canvas_.empty())
    canvas_ = "#canvas";
  TR_INFO("Using the canvas %s", canvas_.c_str());
  //A canvas whose CSS size follows its drawing buffer would grow with
  //every update; it gets the size from engine.ini (or changeSize) in CSS
  //pixels. A canvas the page sizes keeps the page's size. The canvas also
  //gets no context menu (the right button is the game's) and no touch
  //gestures.
  int sized = EM_ASM_INT({
    var canvas = document.querySelector(UTF8ToString($0));
    if (!canvas)
      return -1;
    canvas.addEventListener("contextmenu", function(e){ e.preventDefault(); });
    canvas.style.touchAction = "none";
    var before = canvas.clientWidth;
    canvas.width = canvas.width + 16;
    var follows = canvas.clientWidth != before;
    canvas.width = canvas.width - 16;
    if (follows){
      canvas.style.width = $1 + "px";
      canvas.style.height = $2 + "px";
    }
    return follows ? 0 : 1;
  }, canvas_.c_str(), width_, height_);
  if (sized < 0){
    EXIT2("There is no canvas %s on the page", canvas_.c_str());
    return;
  }
  fixedSize_ = sized == 0;
  updateSize();

  eventWindow = this;
  const char* canvas = canvas_.c_str();
  emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onMouseDown);
  emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onMouseUp);
  emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onMouseMove);
  emscripten_set_wheel_callback(canvas, nullptr, EM_FALSE, onWheel);
  emscripten_set_touchstart_callback(canvas, nullptr, EM_FALSE, onTouch);
  emscripten_set_touchmove_callback(canvas, nullptr, EM_FALSE, onTouch);
  emscripten_set_touchend_callback(canvas, nullptr, EM_FALSE, onTouch);
  emscripten_set_touchcancel_callback(canvas, nullptr, EM_FALSE, onTouch);
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onKeyDown);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onKeyUp);
  emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onFocus);
  emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onFocus);
  emscripten_set_visibilitychange_callback(nullptr, EM_FALSE, onVisibility);
  emscripten_set_webglcontextlost_callback(canvas, nullptr, EM_FALSE, onContextLost);
}

void EmscriptenWindow::kill(){
  if (eventWindow != this)
    return;
  emscripten_html5_remove_all_event_listeners();
  eventWindow = nullptr;
  buttonState = 0;
}

Gfx::NativeWindow EmscriptenWindow::getNativeWindow() const{
  Gfx::NativeWindow wnd;
  wnd.kind = Gfx::NativeWindow::Emscripten;
  wnd.canvas = canvas_.c_str();
  return wnd;
}

float EmscriptenWindow::getContentScale() const{
  return (float)emscripten_get_device_pixel_ratio();
}

void EmscriptenWindow::changeSize(int width, int height){
  if (!fixedSize_)
    return;
  EM_ASM({
    var canvas = document.querySelector(UTF8ToString($0));
    canvas.style.width = $1 + "px";
    canvas.style.height = $2 + "px";
  }, canvas_.c_str(), width, height);
}

void EmscriptenWindow::setTitle(const std::string& title){
  emscripten_set_window_title(title.c_str());
}

void EmscriptenWindow::messageBox(const std::string& title, const std::string& message){
  EM_ASM({
    alert(UTF8ToString($0) + "\n\n" + UTF8ToString($1));
  }, title.c_str(), message.c_str());
}

bool EmscriptenWindow::updateSize(){
  const char* canvas = canvas_.c_str();
  double cssWidth = 0;
  double cssHeight = 0;
  emscripten_get_element_css_size(canvas, &cssWidth, &cssHeight);
  double ratio = emscripten_get_device_pixel_ratio();
  int width = (std::max)(1, (int)std::lround(cssWidth*ratio));
  int height = (std::max)(1, (int)std::lround(cssHeight*ratio));
  left_ = EM_ASM_DOUBLE({return document.querySelector(UTF8ToString($0)).getBoundingClientRect().left;}, canvas);
  top_ = EM_ASM_DOUBLE({return document.querySelector(UTF8ToString($0)).getBoundingClientRect().top;}, canvas);
  scale_ = cssWidth > 0 ? width/cssWidth : ratio;
  int bufferWidth = 0;
  int bufferHeight = 0;
  emscripten_get_canvas_element_size(canvas, &bufferWidth, &bufferHeight);
  if (bufferWidth != width || bufferHeight != height)
    emscripten_set_canvas_element_size(canvas, width, height);
  bool changed = width != width_ || height != height_;
  width_ = width;
  height_ = height;
  return changed;
}

void EmscriptenWindow::toWindow(double clientX, double clientY, int& x, int& y) const{
  x = (int)std::floor((clientX - left_)*scale_);
  y = (int)std::floor((clientY - top_)*scale_);
}

bool EmscriptenWindow::contains(int x, int y) const{
  return x >= 0 && y >= 0 && x < width_ && y < height_;
}

void CGE::installBrowserConsoleTracing(){
  TraceManager::instance()->setTraceOutputter(new BrowserConsoleOutputter());
}

#endif
