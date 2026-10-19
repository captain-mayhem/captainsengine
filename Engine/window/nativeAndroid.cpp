#if defined(ANDROID)
#include <android/native_window.h>
#include "../system/engine.h"
#include "nativeAndroid.h"

using namespace CGE;

TR_CHANNEL_EXT(CGE_Window)

AndroidWindow::AndroidWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc){
}

void AndroidWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");
  //the activity's window; the render device creates its surface on it
  ANativeWindow* wnd = CGE::Engine::instance()->getApp()->window;
  width_ = ANativeWindow_getWidth(wnd);
  height_ = ANativeWindow_getHeight(wnd);
}

void AndroidWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");
}

Gfx::NativeWindow AndroidWindow::getNativeWindow() const{
  Gfx::NativeWindow wnd;
  wnd.kind = Gfx::NativeWindow::Android;
  wnd.window = (uintptr_t)CGE::Engine::instance()->getApp()->window;
  return wnd;
}

#endif
