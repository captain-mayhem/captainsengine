#ifndef NATIVE_LINUX_H
#define NATIVE_LINUX_H

#include <cge_export.h>

#if defined(LINUX)

#include <X11/Xlib.h>
#include "window.h"

namespace CGE{

class CGE_API X11Window : public AppWindow{
public:
  X11Window(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  ~X11Window() {}
  //! init the window
  void init(const std::string& name) override;
  //! get display
  inline Display* getDisplay() const {return disp_;}
  //! get window
  inline Window getWindow() const {return win_;}
  //! get screen
  inline int getScreen() const {return screen_;}
  //! the input context for typed characters (Xutf8LookupString), nullptr if
  //! there is no input method
  inline XIC getInputContext() const {return ic_;}
  Gfx::NativeWindow getNativeWindow() const override;
  //! the desktop's font DPI (Xft.dpi) / 96
  float getContentScale() const override;
  //! kill the window
  void kill() override;
  
  virtual void changeSize(int width, int height) override;
  //! asks the window manager for fullscreen (_NET_WM_STATE_FULLSCREEN)
  void setFullscreen(bool fullscreen) override;
  
  virtual void show(bool doit) override;
  //! is the event the window manager's request to close the window?
  bool isCloseRequest(const XEvent& event) const;
protected:
  //! the window's size in pixels for its windowed size, within the screen
  void windowedPixels(int& width, int& height) const;
  //! the window handle
  Window win_;
  //! the display
  Display* disp_;
  //! the screen number
  int screen_;
  //! the content scale, 0 until it is read
  mutable float contentScale_;
  //! the input method and the window's input context
  XIM im_;
  XIC ic_;
  //! WM_PROTOCOLS and WM_DELETE_WINDOW
  Atom wmProtocols_;
  Atom wmDelete_;
};

}

#endif

#ifdef WIN32
#include "window.h"
namespace CGE{
class CGE_API X11Window : public AppWindow{
public:
  X11Window(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc) {}
  ~X11Window() {}
};
}
#endif

#endif

