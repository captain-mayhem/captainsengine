#ifndef NATIVE_LINUX_H
#define NATIVE_LINUX_H

#include <cge_export.h>

#if defined(LINUX)

#include <X11/Xlib.h>
#ifndef DISABLE_XF86VMODE
#include <X11/extensions/xf86vmode.h>
#endif
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
  Gfx::NativeWindow getNativeWindow() const override;
  //! the desktop's font DPI (Xft.dpi) / 96
  float getContentScale() const override;
  //! kill the window
  void kill() override;
  
  virtual void changeSize(int width, int height) override;
  
  virtual void show(bool doit) override;
protected:
  void applyResolution();
  //! the window handle
  Window win_;
  //! the display
  Display* disp_;
  //! the screen number
  int screen_;
  //! the content scale, 0 until it is read
  mutable float contentScale_;
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
class CGE_API X11Window : public AppWindow{
public:
  X11Window(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc) {}
  ~X11Window() {}
};
}
#endif

#endif

