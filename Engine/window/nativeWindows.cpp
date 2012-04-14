#ifdef WIN32
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "nativeWindows.h"

TR_CHANNEL(CGE_Window);

namespace Windows{

LRESULT CALLBACK messageLoop(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam){
  PAINTSTRUCT ps;
  HDC hdc;

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
      CGE::Engine::instance()->getRenderer()->resizeScene(LOWORD(lparam), HIWORD(lparam));
      return 0;
      break;
    case WM_DESTROY:
      CGE::Engine::instance()->requestShutdown();
      PostQuitMessage(0);
      return 0;
      break;
    case WM_KEYDOWN:
      Input::Keyboard::instance()->keyDown((int)wparam);
      return 0;
      break;
    case WM_KEYUP:
      Input::Keyboard::instance()->keyUp((int)wparam);
      return 0;
      break;
    case WM_CHAR:
      Input::Keyboard::instance()->ascii((unsigned char)wparam);
      return 0;
    case WM_MOUSEMOVE:
      //Input::Mouse::instance()->move((int)LOWORD(lparam), (int)HIWORD(lparam), (int)wparam);
      return 0;
    case WM_LBUTTONDOWN:
      Input::Mouse::instance()->buttonDown((int)LOWORD(lparam), (int)HIWORD(lparam), MK_LBUTTON);
      return 0;
    case WM_MBUTTONDOWN:
      Input::Mouse::instance()->buttonDown((int)LOWORD(lparam), (int)HIWORD(lparam), MK_MBUTTON);
      return 0;
    case WM_RBUTTONDOWN:
      Input::Mouse::instance()->buttonDown((int)LOWORD(lparam), (int)HIWORD(lparam), MK_RBUTTON);
      return 0;
    case WM_LBUTTONUP:
      Input::Mouse::instance()->buttonUp((int)LOWORD(lparam), (int)HIWORD(lparam), MK_LBUTTON);
      return 0;
    case WM_MBUTTONUP:
      Input::Mouse::instance()->buttonUp((int)LOWORD(lparam), (int)HIWORD(lparam), MK_MBUTTON);
      return 0;
    case WM_RBUTTONUP:
      Input::Mouse::instance()->buttonUp((int)LOWORD(lparam), (int)HIWORD(lparam), MK_RBUTTON);
      return 0;
    case WM_LBUTTONDBLCLK:
      Input::Mouse::instance()->doubleClick((int)LOWORD(lparam), (int)HIWORD(lparam), MK_LBUTTON);
      return 0;
    case WM_MBUTTONDBLCLK:
      Input::Mouse::instance()->doubleClick((int)LOWORD(lparam), (int)HIWORD(lparam), MK_MBUTTON);
      return 0;
    case WM_RBUTTONDBLCLK:
      Input::Mouse::instance()->doubleClick((int)LOWORD(lparam), (int)HIWORD(lparam), MK_RBUTTON);
      return 0;
    case WM_MOUSEWHEEL:
      Input::Mouse::instance()->wheel((int)LOWORD(lparam), (int)HIWORD(lparam), GET_WHEEL_DELTA_WPARAM(wparam)/120);
      return 0;
    default:
      break;
  }
  return DefWindowProc(wnd, msg, wparam, lparam);
}

WindowsWindow::WindowsWindow(CGE::Renderer* renderer) : AppWindow(renderer){
  handle_ = NULL;
  instance_ = NULL;
}

WindowsWindow::~WindowsWindow(){

}

void WindowsWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");
#ifndef IDI_APPLICATION
#define IDI_APPLICATION ((LPWSTR)((ULONG_PTR)((WORD)(32512))))
#endif
  HWND hwnd = NULL;
#ifdef UNDER_CE
  WNDCLASS wndclass;
#else
  WNDCLASSEX wndclass;
  wndclass.cbSize = sizeof(WNDCLASSEX);
#endif

  wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS
#ifndef UNDER_CE
    | CS_OWNDC
#endif
    ;
  wndclass.lpfnWndProc = messageLoop;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = GetModuleHandle(NULL);
  wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = TEXT(WINDOW_NAME);
#ifndef UNDER_CE
  wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
#endif

  instance_ = wndclass.hInstance;

#ifdef UNDER_CE
  if (!RegisterClass(&wndclass)){
#else
  if (!RegisterClassEx(&wndclass)){
#endif
    TR_ERROR("Cannot register window");
    EXIT();
  }

  DWORD style = 0;
  DWORD exStyle = 0;

  if (renderer_->getRenderType() == CGE::OpenGL){
    if (fullscreen_){
#ifndef UNDER_CE
      DEVMODE screenSettings;
      memset(&screenSettings, 0, sizeof(screenSettings));
      screenSettings.dmSize = sizeof(screenSettings);
      screenSettings.dmPelsWidth = width_;
      screenSettings.dmPelsHeight = height_;
      screenSettings.dmBitsPerPel = bpp_;
      screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

      if (ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL){
        TR_WARN("Changing to fullscreen failed - Trying windowed mode");
        fullscreen_ = false;
      }
#endif
    }
}

#ifndef UNDER_CE
    if (fullscreen_){
      exStyle = WS_EX_APPWINDOW;
      style = WS_POPUP;
      //ShowCursor(FALSE);
    }
    else{
      exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
      style = WS_OVERLAPPEDWINDOW;
    

    RECT windowRect;
    windowRect.left = (long)0;
    windowRect.right = (long)width_;
    windowRect.top = (long)0;
    windowRect.bottom = (long)height_;

    //Change size
    AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);
  }
#endif

#ifdef UNDER_CE
    wchar_t wtmp[256];
    mbstowcs(wtmp, name.c_str(), 256);
#endif
  if (!(hwnd = CreateWindowEx(exStyle, TEXT(WINDOW_NAME), 
#ifdef UNDER_CE
    wtmp,
#else
    name.c_str(),
#endif
    WS_CLIPCHILDREN | WS_CLIPSIBLINGS | style,
    0, 0, width_, height_, NULL, NULL, instance_, NULL))){
      TR_ERROR("Create Window failed");
      EXIT();
  }

  handle_ = hwnd;

  //take decorations into account
  RECT rectWin;
  RECT rectClient;
  GetWindowRect(handle_, &rectWin);
  GetClientRect(handle_, &rectClient);
  int xoffset = (rectWin.right-rectWin.left)-(rectClient.right-rectClient.left);
  int yoffset = (rectWin.bottom-rectWin.top)-(rectClient.bottom-rectClient.top);
  rectWin.right += xoffset;
  rectWin.bottom += yoffset;
  MoveWindow(handle_, rectWin.left, rectWin.top, rectWin.right-rectWin.left, rectWin.bottom-rectWin.top, TRUE);

  renderer_->initContext(this);

}

void WindowsWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");
  if (fullscreen_){
#ifndef UNDER_CE
    ChangeDisplaySettings(NULL, 0);
#endif
    ShowCursor(TRUE);
  }

  renderer_->killContext();

  if (handle_ && !DestroyWindow(handle_)){
    TR_ERROR("Destroying window failed");
    handle_ = NULL;
  }

  if (!UnregisterClass(TEXT(WINDOW_NAME), instance_)){
    TR_ERROR("Unregistering class failed");
    instance_ = NULL;
  }

}

void WindowsWindow::changeSize(int width, int height){
  AppWindow::changeSize(width, height);
  SetWindowPos(handle_, NULL, 0, 0, width, height, SWP_NOMOVE);
}

}
#endif
