#ifdef QNX
#include "../system/engine.h"
#include "../renderer/GLES/GLESrenderer.h"
#include "nativeQNX.h"

namespace Windows{

QNXWindow::QNXWindow(CGE::Renderer* renderer) : AppWindow(renderer){
  
}

void QNXWindow::init(const std::string& name){
  CGE::Log << "Initializing window\n";

  renderer_->initContext(this);
  renderer_->resizeScene(width_, height_);
}

void QNXWindow::kill(){
  CGE::Log << "Killing window\n";

  renderer_->killContext();
}

}
#endif
