#ifndef NATIVE_WINDOWS_H
#define NATIVE_WINDOWS_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "window.h"

namespace Windows{

class WindowsWindow : public AppWindow{
public:
  WindowsWindow(::Graphics::Renderer* renderer);
  ~WindowsWindow() {}
  //! init the window
  void init(const std::string& name);
  //! get window handle
  inline HWND getHandle(){return handle_;}
  //! kill the window
  void kill();
protected:
  HWND handle_;
  HINSTANCE instance_;
};

}

#endif

#ifdef UNIX
#include "window.h"
namespace Windows{
class WindowsWindow : public AppWindow{
public:
  WindowsWindow(::Renderer::Renderer* renderer) : AppWindow(renderer) {}
  ~WindowsWindow() {}
};
}
#endif

#endif

