#ifndef NATIVE_ANDROID_H
#define NATIVE_ANDROID_H

#if defined(ANDROID)

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
protected:
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
