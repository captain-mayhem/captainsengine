
#include "window.h"

#include "../system/script.h"
#include "../io/Tracing.h"

TR_CHANNEL(CGE_Window);

using namespace Windows;

AppWindow::AppWindow(CGE::Renderer* render){
  renderer_ = render;
  fullscreen_ = CGE::Script::instance()->getBoolSetting("fullscreen");
  width_ = CGE::Script::instance()->getNumberSetting("width");
  if (width_ == 0)
    width_ = 640;
  height_ = CGE::Script::instance()->getNumberSetting("height");
  if (height_ == 0)
    height_ = 480;
  bpp_ = CGE::Script::instance()->getNumberSetting("bitsPerPixel");
  if (bpp_ == 0)
    bpp_ = 32;
}

void AppWindow::focus(bool hasFocus){

}

void AppWindow::messageBox(const std::string& title, const std::string& message){
  TR_USE(CGE_Window);
  TR_WARN("MsgBox failed (not yet implemented): %s: %s", title.c_str(), message.c_str());
}

