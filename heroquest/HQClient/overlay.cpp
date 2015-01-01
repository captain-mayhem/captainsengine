//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | Stefan Ruff                                                      |
// |------------------------------------------------------------------|
// | File: overlay.cpp                                                |
//  ==================================================================

#include "overlay.h"
#ifdef _CLIENT_
  #include "textureManager.h"
  #include "renderer/forms.h"
#endif

//Constructor
Overlay::Overlay(){
  active_ = false;
  rendered_ = false;
  found_ = false;
  walkable_ = true;
  model_ = NULL;
}

//Destructor
Overlay::~Overlay(){
}

Overlay::Overlay(const Overlay& o){
  position_ = o.position_;
  name_ = o.name_;
  id_ = o.id_;
  active_ = o.active_;
  description_ = string(o.description_);
  orientation_ = o.orientation_;
  short_ = string(o.short_);
  width_ = o.width_;
  height_ = o.height_;
  rendered_ = o.rendered_;
  found_ = o.found_;
  walkable_ = o.walkable_;
  model_ = o.model_;
}

void Overlay::render2D() const {
#ifdef _CLIENT_
  if (rendered_ || (!active_ && !found_))
    return;
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
  if (found_){
    rend->setColor(1., 1., 1., 0.4f);
    rend->enableBlend(true);
  }
  int height = wrld.getMapSize().y;
  float dx = (float)(SCREENWIDTH/wrld.getMapSize().x);
  float dy = (float)(SCREENHEIGHT/height);
  rend->enableTexturing(true);
  TextureManager::instance()->overlayTex[id_-4000]->activate();

  switch (orientation_){
  case TOP:
    forms->drawQuad(Vec2f(position_.x*dx, (height - position_.y - height_)*dy), Vec2f(dx*width_, dy*height_));
    break;
  case BOTTOM:
    forms->drawQuad(Vec2f((position_.x - width_ + 1)*dx, (height - (position_.y + 1))*dy), Vec2f(dx*width_, dy*height_), 180);
    break;
  case RIGHT:
    forms->drawQuad(Vec2f((position_.x - height_ + 1)*dx, (height - (position_.y + width_))*dy), Vec2f(dx*width_, dy*height_), 270);
    break;
  case LEFT:
    forms->drawQuad(Vec2f((position_.x)*dx, (height - (position_.y + 1))*dy), Vec2f(dx*width_, dy*height_), 90);
    break;
  }
  if (found_){
    rend->setColor(1., 1., 1., 1.);
    rend->enableBlend(false);
  }
#endif
}

void Overlay::render() const{
#ifdef _CLIENT_
  if (rendered_ || (!active_ && !found_))
    return;
  if (model_){
    model_->setupMaterial();
    model_->render();
    model_->resetMaterial();
  }
#endif
}
