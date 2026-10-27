#ifndef CGE_TESTWINDOW_H
#define CGE_TESTWINDOW_H

#include <renderer/renderdevice.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

// A hidden native window for the OpenGL backend's tests. create() fails
// where no window can be created (Linux without a display), so the tests can
// skip themselves.

#if defined(WIN32) || defined(LINUX)
#define CGE_TEST_WINDOW 1

namespace TestEnv{

//! a hidden native window for a device
class TestWindow{
  typedef CGE::Gfx::RenderDevice RenderDevice;
  typedef CGE::Gfx::DeviceDesc DeviceDesc;
  typedef CGE::Gfx::NativeWindow NativeWindow;
public:
#if defined(WIN32)
  bool create(RenderDevice&, const DeviceDesc&){
    WNDCLASSA wc = {};
    //WGL keeps the device context of a CS_OWNDC window, like the engine's windows
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "CGE_GLContextTest";
    RegisterClassA(&wc);
    wnd_ = CreateWindowA(wc.lpszClassName, "test", WS_OVERLAPPEDWINDOW, 0, 0, 64, 48, nullptr, nullptr, wc.hInstance, nullptr);
    return wnd_ != nullptr;
  }
  NativeWindow native() const{
    NativeWindow window;
    window.kind = NativeWindow::Win32;
    window.window = (uintptr_t)wnd_;
    return window;
  }
  ~TestWindow(){
    if (wnd_)
      DestroyWindow(wnd_);
  }
private:
  HWND wnd_ = nullptr;
#else
  bool create(RenderDevice& device, const DeviceDesc& desc){
    disp_ = XOpenDisplay(nullptr);
    if (!disp_)
      return false;
    int screen = DefaultScreen(disp_);
    XVisualInfo templ = {};
    templ.visualid = device.chooseX11Visual(disp_, screen, desc);
    int count = 0;
    XVisualInfo* visual = XGetVisualInfo(disp_, VisualIDMask, &templ, &count);
    if (!visual)
      return false;
    Window root = RootWindow(disp_, screen);
    XSetWindowAttributes attr = {};
    colormap_ = XCreateColormap(disp_, root, visual->visual, AllocNone);
    attr.colormap = colormap_;
    win_ = XCreateWindow(disp_, root, 0, 0, 64, 48, 0, visual->depth, InputOutput, visual->visual, CWColormap, &attr);
    XFree(visual);
    screen_ = screen;
    return win_ != 0;
  }
  NativeWindow native() const{
    NativeWindow window;
    window.kind = NativeWindow::X11;
    window.display = disp_;
    window.window = (uintptr_t)win_;
    window.screen = screen_;
    return window;
  }
  ~TestWindow(){
    if (win_)
      XDestroyWindow(disp_, win_);
    if (colormap_)
      XFreeColormap(disp_, colormap_);
    if (disp_)
      XCloseDisplay(disp_);
  }
private:
  Display* disp_ = nullptr;
  Window win_ = 0;
  Colormap colormap_ = 0;
  int screen_ = 0;
#endif
};

}

#endif

#endif
