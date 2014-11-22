#ifndef CGE_SPLASH_WINDOW_H
#define CGE_SPLASH_WINDOW_H

#ifdef WIN32
#include <Windows.h>
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
#endif
};

}

#endif
