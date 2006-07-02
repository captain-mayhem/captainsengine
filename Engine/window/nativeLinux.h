#ifndef NATIVE_LINUX_H
#define NATIVE_LINUX_H

#ifdef UNIX

#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>
#include "window.h"

namespace Windows{

class X11Window : public AppWindow{
public:
  X11Window(::Graphics::Renderer* renderer);
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
protected:
  //! the window handle
  Window win_;
  //! the display
  Display* disp_;
  //! the screen
  int screen_;
  //! desktop resolution
  XF86VidModeModeInfo prevDesktop_;
};

}

#endif

#ifdef WIN32
#include "window.h"
namespace Windows{
class X11Window : public AppWindow{
public:
  X11Window(::Graphics::Renderer* renderer) : AppWindow(renderer) {}
  ~X11Window() {}
};
}
#endif

#endif

