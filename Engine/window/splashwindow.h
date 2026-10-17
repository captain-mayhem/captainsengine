#ifndef CGE_SPLASH_WINDOW_H
#define CGE_SPLASH_WINDOW_H

#include <cge_export.h>

#ifdef WIN32
#include <Windows.h>
#elif defined(LINUX)
#include <X11/Xlib.h>
namespace CGE{
class X11Window;
}
#endif

namespace CGE{

class AppWindow;
class Image;

class CGE_API SplashWindow{
public:
  SplashWindow(AppWindow* parent);
  ~SplashWindow();
  void show(Image& img);
private:
#ifdef WIN32
  WNDCLASS m_wc;
  HWND m_window;
#elif defined(LINUX)
  X11Window* m_parent;
  Window m_window;
#endif
};

}

#endif
