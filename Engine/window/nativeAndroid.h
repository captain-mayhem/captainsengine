#ifndef NATIVE_ANDROID_H
#define NATIVE_ANDROID_H

#if defined(ANDROID)

#include <EGL/egl.h>

#include "window.h"

namespace CGE{

class AndroidWindow : public AppWindow{
public:
  AndroidWindow(CGE::Renderer* renderer);
  ~AndroidWindow() {}
  //! init the window
  void init(const std::string& name);
  //! kill the window
  void kill();
  
  EGLDisplay getDisplay() {
    return m_display;
  }
  
  EGLSurface getSurface(){
    return m_surface;
  }
protected:
  EGLDisplay m_display;
  EGLSurface m_surface;
};

}

#endif

#ifdef WIN32
#include "window.h"
namespace CGE{
class AndroidWindow : public AppWindow{
public:
  AndroidWindow(CGE::Renderer* renderer) : AppWindow(renderer) {}
  ~AndroidWindow() {}
};
}
#endif

#endif