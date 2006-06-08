#ifdef WIN32
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"
#include "nativeWindows.h"

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
      ::System::Engine::instance()->getRenderer()->resizeScene(LOWORD(lparam), HIWORD(lparam));
      return 0;
      break;
    case WM_DESTROY:
      ::System::Engine::instance()->shutdown();
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
      Input::Mouse::instance()->move((int)LOWORD(lparam), (int)HIWORD(lparam), (int)wparam);
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
    default:
      break;
  }
  return DefWindowProc(wnd, msg, wparam, lparam);
}

WindowsWindow::WindowsWindow(::Graphics::Renderer* renderer) : AppWindow(renderer){
  handle_ = NULL;
  instance_ = NULL;
}

void WindowsWindow::init(const std::string& name){
  System::Log << "Initializing window\n";
  HWND hwnd = NULL;
  WNDCLASSEX wndclass;

  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wndclass.lpfnWndProc = messageLoop;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = GetModuleHandle(NULL);
  wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = WINDOW_NAME;
  wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  instance_ = wndclass.hInstance;

  if (!RegisterClassEx(&wndclass)){
    System::Log << "Cannot register window\n";
    EXIT();
  }

  DWORD style;
  DWORD exStyle;

  if (renderer_->getRenderType() == ::Graphics::OpenGL){
    if (fullscreen_){
      DEVMODE screenSettings;
      memset(&screenSettings, 0, sizeof(screenSettings));
      screenSettings.dmSize = sizeof(screenSettings);
      screenSettings.dmPelsWidth = width_;
      screenSettings.dmPelsHeight = height_;
      screenSettings.dmBitsPerPel = bpp_;
      screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

      if (ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL){
        System::Log << "Changing to fullscreen failed\nTrying windowed mode\n";
        fullscreen_ = false;
      }
    }
}
    if (fullscreen_){
      exStyle = WS_EX_APPWINDOW;
      style = WS_POPUP;
      ShowCursor(FALSE);
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

  if (!(hwnd = CreateWindowEx(exStyle, WINDOW_NAME, name.c_str(), WS_CLIPCHILDREN | WS_CLIPSIBLINGS | style,
    0, 0, width_, height_, NULL, NULL, instance_, NULL))){
      System::Log << "Create Window failed" << "\n";
      EXIT();
  }

  handle_ = hwnd;

  renderer_->initContext(this);

}

void WindowsWindow::kill(){
  ::System::Log << "Killing window\n";
  if (fullscreen_){
    ChangeDisplaySettings(NULL, 0);
    ShowCursor(TRUE);
  }

  renderer_->killContext();

  if (handle_ && !DestroyWindow(handle_)){
    ::System::Log << "Destroying window failed";
    handle_ = NULL;
  }

  if (!UnregisterClass(WINDOW_NAME, instance_)){
    ::System::Log << "Unregistering class failed";
    instance_ = NULL;
  }

}

}
#endif
