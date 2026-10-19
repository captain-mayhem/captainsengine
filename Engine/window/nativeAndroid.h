#ifndef NATIVE_ANDROID_H
#define NATIVE_ANDROID_H

#include <cge_export.h>

#if defined(ANDROID)

#include "window.h"

namespace CGE{

class CGE_API AndroidWindow : public AppWindow{
public:
  AndroidWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc);
  ~AndroidWindow() {}
  //! init the window
  void init(const std::string& name) override;
  //! kill the window
  void kill() override;
  Gfx::NativeWindow getNativeWindow() const override;
};

}

#endif

#ifdef WIN32
#include "window.h"
namespace CGE{
class CGE_API AndroidWindow : public AppWindow{
public:
  AndroidWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc) {}
  ~AndroidWindow() {}
};
}
#endif

#endif
