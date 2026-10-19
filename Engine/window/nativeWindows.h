#ifndef NATIVE_WINDOWS_H
#define NATIVE_WINDOWS_H

#include <cge_export.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "window.h"

namespace CGE{

class CGE_API WindowsWindow : public AppWindow{
public:
  WindowsWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  ~WindowsWindow();
  //! init the window
  void init(const std::string& name) override;
  //! get window handle
  inline void* getHandle() override {return handle_;}
  Gfx::NativeWindow getNativeWindow() const override;
  //! kill the window
  void kill() override;
  virtual void changeSize(int width, int height) override;

  virtual void messageBox(const std::string& title, const std::string& message) override;

  virtual void show(bool doit) override;

  virtual void setTitle(const std::string& title) override;
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
class CGE_API WindowsWindow : public AppWindow{
public:
  WindowsWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc) {}
  ~WindowsWindow() {}
};
}
#endif

#endif

