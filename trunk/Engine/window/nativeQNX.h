#ifndef NATIVE_QNX_H
#define NATIVE_QNX_H

#if defined(QNX) || defined(NO_X11)

#include "window.h"

namespace CGE{

class QNXWindow : public AppWindow{
public:
  QNXWindow(CGE::Renderer* renderer);
  ~QNXWindow() {}
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
class QNXWindow : public AppWindow{
public:
  QNXWindow(CGE::Renderer* renderer) : AppWindow(renderer) {}
  ~QNXWindow() {}
};
}
#endif

#endif

