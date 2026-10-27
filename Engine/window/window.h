#ifndef WINDOW_H
#define WINDOW_H

#include <cge_export.h>

#include <string>

#include "../renderer/gfxtypes.h"

namespace CGE{
namespace Gfx{
class RenderDevice;
}
}

namespace CGE{

#define WINDOW_NAME "Captains Game Engine"

//! A native window
/*! It only provides native handles (getNativeWindow); the render device
    creates the graphics context on it. */
class CGE_API AppWindow{
public:
  //! the window is created for device, which gets desc in its init
  AppWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  virtual ~AppWindow(){};
  virtual void init(const std::string& name){};
  virtual void kill(){};
  //! get Bits per pixel
  inline const int getBpp() const {return bpp_;}
  //! is window fullscreen?
  inline const bool isFullscreen() const {return fullscreen_;}
  //! switches between a window and borderless fullscreen on the window's monitor
  /*! Windows and Linux; Alt+Enter toggles it there. The window's size
      follows with the resize event. Elsewhere it does nothing. */
  virtual void setFullscreen(bool fullscreen) {}
  //! get window width in pixels
  inline const int getWidth() const {return width_;}
  //! set window width
  inline void setWidth(int width) {width_ = width;}
  //! get window height in pixels
  inline const int getHeight() const {return height_;}
  //! set window height
  inline void setHeight(int height) {height_ = height;}
  //! set focussed
  void focus(bool hasFocus);
	//! get a native window handle
	virtual void* getHandle() {return nullptr;}
  //! the native handles for the render device
  virtual Gfx::NativeWindow getNativeWindow() const {return Gfx::NativeWindow();}
  //! the size of the window's contents relative to 96 DPI (1 = no scaling)
  virtual float getContentScale() const {return 1;}

  //! sets the size of the window's contents in logical pixels
  /*! Logical pixels are pixels at 96 DPI (CSS pixels on the web), as are
      width and height in engine.ini; the window scales them with
      getContentScale and keeps the window on its screen. In fullscreen the
      size is used when the window leaves it. getWidth and getHeight
      follow when the window reports its new size. */
  virtual void changeSize(int width, int height){
    windowedWidth_ = width;
    windowedHeight_ = height;
    width_ = width;
    height_ = height;
  }

  virtual void messageBox(const std::string& title, const std::string& message);

  virtual void show(bool doit){
    shown_ = doit;
  }

  virtual void setTitle(const std::string& title) {}
protected:
  Gfx::RenderDevice* device_;
  Gfx::DeviceDesc deviceDesc_;
  bool fullscreen_;
  //! the size in pixels
  int width_;
  int height_;
  //! the size outside fullscreen in logical pixels (changeSize)
  int windowedWidth_;
  int windowedHeight_;
  int bpp_;
  bool shown_;
};

}

#endif

