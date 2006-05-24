#include "../system/script.h"
#include "window.h"


namespace Windows{

AppWindow::AppWindow(::Graphics::Renderer* render){
  renderer_ = render;
  fullscreen_ = ::System::Script::instance()->getBoolSetting("fullscreen");
  width_ = ::System::Script::instance()->getNumberSetting("width");
  height_ = ::System::Script::instance()->getNumberSetting("height");
  bpp_ = ::System::Script::instance()->getNumberSetting("bitsPerPixel");
}

}
