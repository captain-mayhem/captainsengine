#ifndef NATIVE_LINUX_H
#define NATIVE_LINUX_H

#if defined(LINUX) && !defined(NO_X11)

#include <X11/Xlib.h>
#ifndef DISABLE_XF86VMODE
#include <X11/extensions/xf86vmode.h>
#endif
#include "window.h"

namespace CGE{

class X11Window : public AppWindow{
public:
  X11Window(CGE::Renderer* renderer);
  ~X11Window() {}
  //! init the window
  void init(const std::string& name);
  //! get display
  inline Display* getDisplay() const {return disp_;}
  //! get window
  inline Window getWindow() const {return win_;}
  //! get screen
  inline int getScreen() const {return screen_;}
  //! kill the window
  void kill();
  
  virtual void changeSize(int width, int height);
protected:
  //! the window handle
  Window win_;
  //! the display
  Display* disp_;
  //! the screen
  int screen_;
#ifndef DISABLE_XF86VMODE
  //! desktop resolution
  XF86VidModeModeInfo prevDesktop_;
#endif
};

}

#endif

#ifdef WIN32
#include "window.h"
namespace CGE{
class X11Window : public AppWindow{
public:
  X11Window(CGE::Renderer* renderer) : AppWindow(renderer) {}
  ~X11Window() {}
};
}
#endif

#endif

