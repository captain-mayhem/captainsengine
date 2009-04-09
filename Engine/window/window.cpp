#include "../system/script.h"
#include "window.h"


namespace Windows{

AppWindow::AppWindow(CGE::Renderer* render){
  renderer_ = render;
  fullscreen_ = CGE::Script::instance()->getBoolSetting("fullscreen");
  width_ = CGE::Script::instance()->getNumberSetting("width");
  height_ = CGE::Script::instance()->getNumberSetting("height");
  bpp_ = CGE::Script::instance()->getNumberSetting("bitsPerPixel");
}

}
