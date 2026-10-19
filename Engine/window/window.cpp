
#include "window.h"

#include "../system/script.h"
#include "../io/Tracing.h"

using namespace CGE;

TR_CHANNEL(CGE_Window);

AppWindow::AppWindow(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc){
  device_ = device;
  deviceDesc_ = desc;
  fullscreen_ = CGE::Script::instance()->getBoolSetting("fullscreen");
  width_ = CGE::Script::instance()->getNumberSetting("width");
  if (width_ == 0)
    width_ = 640;
  height_ = CGE::Script::instance()->getNumberSetting("height");
  if (height_ == 0)
    height_ = 480;
  bpp_ = desc.colorBits;
  bool shownSet;
  shown_ = CGE::Script::instance()->getBoolSetting("windowShown", &shownSet);
  if (!shownSet)
    shown_ = true;
}

void AppWindow::focus(bool hasFocus){

}

void AppWindow::messageBox(const std::string& title, const std::string& message){
  TR_USE(CGE_Window);
  TR_WARN("MsgBox failed (not yet implemented): %s: %s", title.c_str(), message.c_str());
}

