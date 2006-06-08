#ifndef WINDOW_H
#define WINDOW_H

#include <string>

namespace Graphics{
class Renderer;
}

namespace Windows{

#define WINDOW_NAME "Hero-Engine"

class AppWindow{
public:
  AppWindow(::Graphics::Renderer* render);
  virtual ~AppWindow(){};
  virtual void init(const std::string& name){};
  virtual void kill(){};
  //! get Bits per pixel
  inline const int getBpp() const {return bpp_;}
  //! is window fullscreen?
  inline const bool isFullscreen() const {return fullscreen_;}
  //! get window width
  inline const int getWidth() const {return width_;}
  //! set window width
  inline void setWidth(int width) {width_ = width;}
  //! get window height
  inline const int getHeight() const {return height_;}
  //! set window height
  inline void setHeight(int height) {height_ = height;}
protected:
  ::Graphics::Renderer* renderer_;
  bool fullscreen_;
  int width_;
  int height_;
  int bpp_;
};

}

#endif
