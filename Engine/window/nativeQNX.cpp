#if defined(QNX) || defined(NO_X11)
#include "../system/engine.h"
#include "../renderer/GLES/GLESrenderer.h"
#include "nativeQNX.h"

using namespace CGE;

TR_CHANNEL_EXT(CGE_Window)

QNXWindow::QNXWindow(CGE::Renderer* renderer) : AppWindow(renderer){
  
}

void QNXWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");

  renderer_->initContext(this);
  renderer_->resizeScene(width_, height_);
}

void QNXWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");

  renderer_->killContext();
}

#endif
