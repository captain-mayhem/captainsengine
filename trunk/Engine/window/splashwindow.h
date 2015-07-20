#ifndef CGE_SPLASH_WINDOW_H
#define CGE_SPLASH_WINDOW_H

#ifdef WIN32
#include <Windows.h>
#elif defined(LINUX) && !defined(NO_X11)
#include <X11/Xlib.h>
namespace CGE{
class X11Window;
}
#endif

namespace CGE{

class AppWindow;
class Image;

class SplashWindow{
public:
  SplashWindow(AppWindow* parent);
  ~SplashWindow();
  void show(Image& img);
private:
#ifdef WIN32
  WNDCLASS m_wc;
  HWND m_window;
#elif defined(LINUX) && !defined(NO_X11)
  X11Window* m_parent;
  Window m_window;
#endif
};

}

#endif
