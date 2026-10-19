#ifndef CGE_GL_CONTEXT_H
#define CGE_GL_CONTEXT_H

#include <memory>

#include "../gfxtypes.h"

namespace CGE{
namespace Gfx{

//! An OpenGL context on a window, created by the GL backend
/*! One implementation per platform: WGL (Windows), GLX (Linux/X11) and EGL
    (Android). */
class GLContext{
public:
  //! the context implementation of this platform
  static std::unique_ptr<GLContext> create();
  virtual ~GLContext() {}
  //! X11 only: see RenderDevice::chooseX11Visual
  virtual unsigned long chooseX11Visual(void* display, int screen, const DeviceDesc& desc) {return 0;}
  //! creates the context, makes it current and returns the drawable size
  virtual bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) = 0;
  //! releases and destroys the context
  virtual void shutdown() = 0;
  virtual void swapBuffers() = 0;
  //! 1 waits for the vertical blank, 0 doesn't
  virtual void setSwapInterval(int interval) = 0;
};

}
}

#endif
