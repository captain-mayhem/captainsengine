#ifdef WIN32
#include "../system/engine.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "nativeWindows.h"
#include "../platform/platform_internal.h"
#include "../ui/imgui_impl_cge_platform.h"

#include <windowsx.h>
#include <algorithm>
#include <cstring>

TR_CHANNEL_EXT(CGE_Window);

using namespace CGE;

namespace{

const DWORD WINDOWED_STYLE = WS_OVERLAPPEDWINDOW;
const DWORD WINDOW_EX_STYLE = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
//! the timer that renders while the window is moved or sized
const UINT_PTR SIZE_MOVE_TIMER = 1;

//! the window rectangle around a client area of width x height logical pixels
RECT windowRect(int width, int height, UINT dpi){
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = MulDiv(width, dpi, 96);
  rect.bottom = MulDiv(height, dpi, 96);
  AdjustWindowRectExForDpi(&rect, WINDOWED_STYLE, FALSE, WINDOW_EX_STYLE, dpi);
  return rect;
}

WindowsWindow* engineWindow(){
  Engine* engine = Engine::instance();
  return engine ? static_cast<WindowsWindow*>(engine->getWindow()) : nullptr;
}

//! passes a key and the modifier state on to ImGui
void imguiKey(WPARAM key, bool down){
  ImGui_ImplCGE_Platform_AddModifiers(GetKeyState(VK_CONTROL) < 0, GetKeyState(VK_SHIFT) < 0,
    GetKeyState(VK_MENU) < 0, GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0);
  ImGui_ImplCGE_Platform_AddKey((int)key, down);
}

//! passes a mouse button event on to ImGui, at the position it happened
void imguiButton(LPARAM pos, int button, bool down){
  ImGui_ImplCGE_Platform_AddMousePos((float)GET_X_LPARAM(pos), (float)GET_Y_LPARAM(pos));
  ImGui_ImplCGE_Platform_AddMouseButton(button, down);
}

}

LRESULT CALLBACK messageLoop(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam){
  PAINTSTRUCT ps;
  HDC hdc;
  //Windows runs its own loop while the window is moved or sized
  static bool inSizeMove = false;

  //process messages
  switch(msg){
    case WM_CREATE:
      return 0;
      break;
    case WM_PAINT:
      hdc = BeginPaint(wnd, &ps);
      EndPaint(wnd, &ps);
      return 0;
      break;
    case WM_SIZE:
      if (wparam == SIZE_MINIMIZED){
        CGE::Engine::instance()->pause();
        return 0;
      }
      CGE::Engine::instance()->resume();
      CGE::Engine::instance()->resize(LOWORD(lparam), HIWORD(lparam));
      //draw the new size right away instead of stretching the old frame
      if (inSizeMove)
        CGE::Engine::instance()->runModalFrame();
      return 0;
      break;
    case WM_ENTERSIZEMOVE:
      inSizeMove = true;
      SetTimer(wnd, SIZE_MOVE_TIMER, USER_TIMER_MINIMUM, nullptr);
      return 0;
    case WM_EXITSIZEMOVE:
      inSizeMove = false;
      KillTimer(wnd, SIZE_MOVE_TIMER);
      return 0;
    case WM_TIMER:
      if (wparam == SIZE_MOVE_TIMER){
        CGE::Engine::instance()->runModalFrame();
        return 0;
      }
      break;
    case WM_DPICHANGED:
      if (WindowsWindow* window = engineWindow())
        window->dpiChanged(*(RECT*)lparam);
      return 0;
    case WM_SETCURSOR:
      //the cursor chosen with Platform::setCursor in the client area
      if (LOWORD(lparam) == HTCLIENT){
        CGE::PlatformInternal::applyCursor();
        return TRUE;
      }
      break;
    case WM_DESTROY:
      CGE::Engine::instance()->requestShutdown();
      PostQuitMessage(0);
      return 0;
      break;
    case WM_KEYDOWN:
      imguiKey(wparam, true);
      Input::Keyboard::instance()->keyDown((int)wparam);
      return 0;
      break;
    case WM_KEYUP:
      imguiKey(wparam, false);
      Input::Keyboard::instance()->keyUp((int)wparam);
      return 0;
      break;
    //Alt and F10 (only for ImGui); DefWindowProc keeps Alt+F4 working
    case WM_SYSKEYDOWN:
      //Alt+Enter toggles fullscreen
      if (wparam == VK_RETURN && (lparam & (1 << 29))){
        WindowsWindow* window = engineWindow();
        if (window && !(lparam & (1 << 30)))
          window->setFullscreen(!window->isFullscreen());
        return 0;
      }
      imguiKey(wparam, true);
      break;
    //Alt+Enter would beep
    case WM_SYSCHAR:
      if (wparam == VK_RETURN)
        return 0;
      break;
    case WM_SYSKEYUP:
      imguiKey(wparam, false);
      break;
    case WM_SETFOCUS:
      ImGui_ImplCGE_Platform_AddFocus(true);
      break;
    case WM_KILLFOCUS:
      ImGui_ImplCGE_Platform_AddFocus(false);
      break;
    case WM_CHAR:
      Input::Keyboard::instance()->ascii((unsigned char)wparam);
      return 0;
    case WM_MOUSEMOVE:
      //Input::Mouse::instance()->move((int)LOWORD(lparam), (int)HIWORD(lparam), (int)wparam);
      return 0;
    case WM_LBUTTONDOWN:
      imguiButton(lparam, ImGuiMouseButton_Left, true);
      Input::Mouse::instance()->buttonDown((int)LOWORD(lparam), (int)HIWORD(lparam), MK_LBUTTON);
      return 0;
    case WM_MBUTTONDOWN:
      imguiButton(lparam, ImGuiMouseButton_Middle, true);
      Input::Mouse::instance()->buttonDown((int)LOWORD(lparam), (int)HIWORD(lparam), MK_MBUTTON);
      return 0;
    case WM_RBUTTONDOWN:
      imguiButton(lparam, ImGuiMouseButton_Right, true);
      Input::Mouse::instance()->buttonDown((int)LOWORD(lparam), (int)HIWORD(lparam), MK_RBUTTON);
      return 0;
    case WM_LBUTTONUP:
      imguiButton(lparam, ImGuiMouseButton_Left, false);
      Input::Mouse::instance()->buttonUp((int)LOWORD(lparam), (int)HIWORD(lparam), MK_LBUTTON);
      return 0;
    case WM_MBUTTONUP:
      imguiButton(lparam, ImGuiMouseButton_Middle, false);
      Input::Mouse::instance()->buttonUp((int)LOWORD(lparam), (int)HIWORD(lparam), MK_MBUTTON);
      return 0;
    case WM_RBUTTONUP:
      imguiButton(lparam, ImGuiMouseButton_Right, false);
      Input::Mouse::instance()->buttonUp((int)LOWORD(lparam), (int)HIWORD(lparam), MK_RBUTTON);
      return 0;
    //ImGui detects double clicks itself
    case WM_LBUTTONDBLCLK:
      imguiButton(lparam, ImGuiMouseButton_Left, true);
      Input::Mouse::instance()->doubleClick((int)LOWORD(lparam), (int)HIWORD(lparam), MK_LBUTTON);
      return 0;
    case WM_MBUTTONDBLCLK:
      imguiButton(lparam, ImGuiMouseButton_Middle, true);
      Input::Mouse::instance()->doubleClick((int)LOWORD(lparam), (int)HIWORD(lparam), MK_MBUTTON);
      return 0;
    case WM_RBUTTONDBLCLK:
      imguiButton(lparam, ImGuiMouseButton_Right, true);
      Input::Mouse::instance()->doubleClick((int)LOWORD(lparam), (int)HIWORD(lparam), MK_RBUTTON);
      return 0;
    case WM_MOUSEWHEEL:
      ImGui_ImplCGE_Platform_AddMouseWheel(0, (float)GET_WHEEL_DELTA_WPARAM(wparam)/WHEEL_DELTA);
      Input::Mouse::instance()->wheel((int)LOWORD(lparam), (int)HIWORD(lparam), GET_WHEEL_DELTA_WPARAM(wparam)/120);
      return 0;
    case WM_MOUSEHWHEEL:
      ImGui_ImplCGE_Platform_AddMouseWheel(-(float)GET_WHEEL_DELTA_WPARAM(wparam)/WHEEL_DELTA, 0);
      return 0;
    default:
      break;
  }
  return DefWindowProc(wnd, msg, wparam, lparam);
}

WindowsWindow::WindowsWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc){
  handle_ = nullptr;
  instance_ = nullptr;
  memset(&windowedPlacement_, 0, sizeof(windowedPlacement_));
}

WindowsWindow::~WindowsWindow(){

}

void WindowsWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");
  //per-monitor DPI aware (V2: the non-client area scales too), so that the
  //window gets its size in pixels and WM_DPICHANGED instead of being scaled
  //up blurry; fails harmlessly if the application's manifest set it already
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#ifndef IDI_APPLICATION
#define IDI_APPLICATION ((LPWSTR)((ULONG_PTR)((WORD)(32512))))
#endif
  HWND hwnd = nullptr;
  WNDCLASSEX wndclass;
  wndclass.cbSize = sizeof(WNDCLASSEX);

  wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS
    | CS_OWNDC
    ;
  wndclass.lpfnWndProc = messageLoop;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = GetModuleHandle(nullptr);
  wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszMenuName = nullptr;
  wndclass.lpszClassName = TEXT(WINDOW_NAME);
  wndclass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

  instance_ = wndclass.hInstance;

  if (!RegisterClassEx(&wndclass)){
    TR_ERROR("Cannot register window");
    EXIT();
  }

  //created at the default position, then sized for its monitor's DPI
  if (!(hwnd = CreateWindowEx(WINDOW_EX_STYLE, TEXT(WINDOW_NAME),
    name.c_str(),
    WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WINDOWED_STYLE,
    CW_USEDEFAULT, CW_USEDEFAULT, windowedWidth_, windowedHeight_, nullptr, nullptr, instance_, nullptr))){
      TR_ERROR("Create Window failed");
      EXIT();
  }

  handle_ = hwnd;
  placeWindow();
  if (fullscreen_){
    fullscreen_ = false;
    setFullscreen(true);
  }

  if (shown_)
    show(true);

}

void WindowsWindow::placeWindow(){
  MONITORINFO monitor;
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfo(MonitorFromWindow(handle_, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& work = monitor.rcWork;
  RECT rect = windowRect(windowedWidth_, windowedHeight_, GetDpiForWindow(handle_));
  //no larger than the work area
  int width = (std::min)(rect.right-rect.left, work.right-work.left);
  int height = (std::min)(rect.bottom-rect.top, work.bottom-work.top);
  int x = work.left + (work.right-work.left-width)/2;
  int y = work.top + (work.bottom-work.top-height)/2;
  //the size goes to Engine::resize with WM_SIZE
  SetWindowPos(handle_, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
  RECT client;
  GetClientRect(handle_, &client);
  width_ = client.right-client.left;
  height_ = client.bottom-client.top;
}

void WindowsWindow::fitToMonitor(){
  MONITORINFO monitor;
  monitor.cbSize = sizeof(monitor);
  GetMonitorInfo(MonitorFromWindow(handle_, MONITOR_DEFAULTTONEAREST), &monitor);
  const RECT& rect = monitor.rcMonitor;
  SetWindowPos(handle_, HWND_TOP, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
}

void WindowsWindow::setFullscreen(bool fullscreen){
  if (!handle_ || fullscreen == fullscreen_){
    fullscreen_ = fullscreen;
    return;
  }
  fullscreen_ = fullscreen;
  DWORD style = GetWindowLong(handle_, GWL_STYLE);
  if (fullscreen){
    windowedPlacement_.length = sizeof(windowedPlacement_);
    GetWindowPlacement(handle_, &windowedPlacement_);
    SetWindowLong(handle_, GWL_STYLE, (style & ~WINDOWED_STYLE) | WS_POPUP);
    fitToMonitor();
  }
  else{
    SetWindowLong(handle_, GWL_STYLE, (style & ~WS_POPUP) | WINDOWED_STYLE);
    //a hidden window stays hidden
    if (!IsWindowVisible(handle_))
      windowedPlacement_.showCmd = SW_HIDE;
    SetWindowPlacement(handle_, &windowedPlacement_);
    SetWindowPos(handle_, nullptr, 0, 0, 0, 0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

void WindowsWindow::dpiChanged(const RECT& suggested){
  if (fullscreen_){
    fitToMonitor();
    return;
  }
  SetWindowPos(handle_, nullptr, suggested.left, suggested.top, suggested.right-suggested.left,
    suggested.bottom-suggested.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

Gfx::NativeWindow WindowsWindow::getNativeWindow() const{
  Gfx::NativeWindow wnd;
  wnd.kind = Gfx::NativeWindow::Win32;
  wnd.window = (uintptr_t)handle_;
  return wnd;
}

float WindowsWindow::getContentScale() const{
  if (!handle_)
    return 1;
  return GetDpiForWindow(handle_)/96.0f;
}

void WindowsWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");
  if (handle_ && !DestroyWindow(handle_)){
    TR_ERROR("Destroying window failed");
    handle_ = nullptr;
  }

  if (!UnregisterClass(TEXT(WINDOW_NAME), instance_)){
    TR_ERROR("Unregistering class failed");
    instance_ = nullptr;
  }

}

void WindowsWindow::changeSize(int width, int height){
  windowedWidth_ = width;
  windowedHeight_ = height;
  if (!handle_)
    return;
  if (!fullscreen_){
    placeWindow();
    return;
  }
  //the size of the window when it leaves fullscreen, about the same centre
  RECT rect = windowRect(width, height, GetDpiForWindow(handle_));
  RECT& normal = windowedPlacement_.rcNormalPosition;
  int x = (normal.left+normal.right)/2 - (rect.right-rect.left)/2;
  int y = (normal.top+normal.bottom)/2 - (rect.bottom-rect.top)/2;
  normal.left = x;
  normal.top = y;
  normal.right = x+rect.right-rect.left;
  normal.bottom = y+rect.bottom-rect.top;
}

void WindowsWindow::messageBox(const std::string& title, const std::string& message){
  MessageBox(handle_, message.c_str(), title.c_str(), MB_OK);
}

void WindowsWindow::show(bool doit){
  if (doit){
    ShowWindow(handle_, SW_SHOW);
    SetForegroundWindow(handle_);
    SetFocus(handle_);
  }
  else
    ShowWindow(handle_, SW_HIDE);
  AppWindow::show(doit);
}

void WindowsWindow::setTitle(const std::string& title){
  SetWindowText(handle_, title.c_str());
}

#endif
