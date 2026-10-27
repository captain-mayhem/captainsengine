#include "virtualscreen.h"

#include <algorithm>
#include <cmath>

#include "../renderer/renderdevice.h"

using namespace CGE;

VirtualScreen::VirtualScreen() : width_(1024), height_(768), windowWidth_(1024), windowHeight_(768), scale_(1){
}

void VirtualScreen::setSize(int width, int height){
  width_ = (std::max)(width, 1);
  height_ = (std::max)(height, 1);
  update();
}

void VirtualScreen::setWindowSize(int width, int height){
  if (width == windowWidth_ && height == windowHeight_)
    return;
  windowWidth_ = (std::max)(width, 1);
  windowHeight_ = (std::max)(height, 1);
  update();
}

void VirtualScreen::update(){
  scale_ = (std::min)((float)windowWidth_/width_, (float)windowHeight_/height_);
  offset_.x = (windowWidth_-width_*scale_)/2;
  offset_.y = (windowHeight_-height_*scale_)/2;
}

Vec2f VirtualScreen::getVisibleMin() const{
  return Vec2f(-offset_.x/scale_, -offset_.y/scale_);
}

Vec2f VirtualScreen::getVisibleMax() const{
  return Vec2f((windowWidth_-offset_.x)/scale_, (windowHeight_-offset_.y)/scale_);
}

Vec2i VirtualScreen::toVirtual(int x, int y) const{
  return Vec2i((int)std::floor((x-offset_.x)/scale_), (int)std::floor((y-offset_.y)/scale_));
}

Vec2i VirtualScreen::toWindow(int x, int y) const{
  return Vec2i((int)std::lround(x*scale_+offset_.x), (int)std::lround(y*scale_+offset_.y));
}

Matrix VirtualScreen::ortho(const Gfx::RenderDevice& device, bool yDown) const{
  Vec2f vmin = getVisibleMin();
  Vec2f vmax = getVisibleMax();
  //the area is centred, so flipping y maps the visible range onto itself
  if (yDown)
    return device.ortho(vmin.x, vmax.x, vmax.y, vmin.y, -1.0f, 1.0f);
  return device.ortho(vmin.x, vmax.x, height_-vmax.y, height_-vmin.y, -1.0f, 1.0f);
}
