#ifndef NATIVE_WINDOWS_H
#define NATIVE_WINDOWS_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "window.h"

namespace CGE{

class WindowsWindow : public AppWindow{
public:
  WindowsWindow(CGE::Renderer* renderer);
  ~WindowsWindow();
  //! init the window
  void init(const std::string& name);
  //! get window handle
  inline void* getHandle(){return handle_;}
  //! kill the window
  void kill();
  virtual void changeSize(int width, int height);

  virtual void messageBox(const std::string& title, const std::string& message);

  virtual void show(bool doit);

  virtual void setTitle(const std::string& title);
protected:
  void applyResolution();
  HWND handle_;
  HINSTANCE instance_;
};

}

#endif

#ifdef UNIX
#include "window.h"
namespace CGE{
class WindowsWindow : public AppWindow{
public:
  WindowsWindow(CGE::Renderer* renderer) : AppWindow(renderer) {}
  ~WindowsWindow() {}
};
}
#endif

#endif

