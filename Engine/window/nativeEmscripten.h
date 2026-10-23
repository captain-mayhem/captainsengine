#ifndef NATIVE_EMSCRIPTEN_H
#define NATIVE_EMSCRIPTEN_H

#include <cge_export.h>

#if defined(__EMSCRIPTEN__)

#include <string>

#include "window.h"

namespace CGE{

//! The browser's canvas as the engine's window (emscripten/html5.h)
/*! The page decides the canvas's size with CSS, normally the whole browser
    window. Its drawing buffer has that size times devicePixelRatio, so the
    window's pixels are the screen's pixels. The browser delivers mouse,
    touch and keyboard events between frames; they go to the engine's input
    and ImGui like the other platforms' events. A single touch acts as the
    left mouse button. */
class CGE_API EmscriptenWindow : public AppWindow{
public:
  EmscriptenWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  ~EmscriptenWindow() {}
  //! takes the canvas (canvas in engine.ini, by default #canvas) and installs the event handlers
  void init(const std::string& name) override;
  //! removes the event handlers
  void kill() override;
  Gfx::NativeWindow getNativeWindow() const override;
  //! the browser's devicePixelRatio
  float getContentScale() const override;
  //! the page sizes the canvas, so this only remembers the size the application wants
  void changeSize(int width, int height) override;
  //! the page's title
  void setTitle(const std::string& title) override;
  //! window.alert
  void messageBox(const std::string& title, const std::string& message) override;
  //! the CSS selector of the canvas
  const std::string& getCanvas() const {return canvas_;}
  //! sizes the drawing buffer to the canvas's CSS size and devicePixelRatio
  /*! Returns true if the size changed. The engine calls it every frame,
      which also covers zooming and moving to a screen with another
      resolution, where the browser sends no resize event. */
  bool updateSize();
  //! window pixels of a point in CSS pixels relative to the page's viewport
  void toWindow(double clientX, double clientY, int& x, int& y) const;
  //! is the point (window pixels) on the canvas?
  bool contains(int x, int y) const;
protected:
  std::string canvas_;
  //! the canvas's position in the page's viewport (CSS pixels), from updateSize
  double left_;
  double top_;
  //! window pixels per CSS pixel
  double scale_;
  //! the page doesn't size the canvas, so the window sets its CSS size
  bool fixedSize_;
};

//! sends the engine's tracing to the browser's console
CGE_API void installBrowserConsoleTracing();

}

#endif

#endif
