#ifndef NATIVE_QNX_H
#define NATIVE_QNX_H

#ifdef QNX

#include "window.h"

namespace Windows{

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
namespace Windows{
class QNXWindow : public AppWindow{
public:
  QNXWindow(CGE::Renderer* renderer) : AppWindow(renderer) {}
  ~QNXWindow() {}
};
}
#endif

#endif

