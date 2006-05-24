#include "../system/script.h"
#include "window.h"


namespace Windows{

AppWindow::AppWindow(::Renderer::Renderer* render){
  renderer_ = render;
  fullscreen_ = ::Engine::Script::instance()->getBoolSetting("fullscreen");
  width_ = ::Engine::Script::instance()->getNumberSetting("width");
  height_ = ::Engine::Script::instance()->getNumberSetting("height");
  bpp_ = ::Engine::Script::instance()->getNumberSetting("bitsPerPixel");
}

}
