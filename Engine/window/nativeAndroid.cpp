#if defined(ANDROID)
#include "../system/engine.h"
#include "../renderer/GL2/GL2Renderer.h"
#include "nativeAndroid.h"

using namespace CGE;

TR_CHANNEL_EXT(CGE_Window)

AndroidWindow::AndroidWindow(CGE::Renderer* renderer) : AppWindow(renderer){
  
}

void AndroidWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");

  renderer_->initContext(this);
  renderer_->resizeScene(width_, height_);
}

void AndroidWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");

  renderer_->killContext();
}

#endif