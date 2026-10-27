//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Stefan Ruff                                                      |
// | sisnruff@stud.informatik.uni-erlangen.de                         |
// |------------------------------------------------------------------|
// | File: furniture.cpp                                              |
//  ==================================================================

#include "furniture.h"
#include "common.h"
#include "world.h"
#ifdef _CLIENT_
	#include "textureManager.h"
#include "renderer/forms.h"
#include "renderer/drawcontext.h"
#endif


Furniture::Furniture(const Furniture& f){
  position_ = f.position_;
  id_ = f.id_;
  active_ = f.active_;
  interpolate_ = f.interpolate_;

	orientation_ = f.orientation_;
	brev_ = string(f.brev_);
	width_ = f.width_;
	height_ = f.height_;
	rendered_ = f.rendered_;
  mModel = f.mModel;
}

void Furniture::render() const {
#ifdef _CLIENT_
	if (rendered_)
		return;

  if (mModel)
    mModel->render(*CGE::Engine::instance()->getDrawContext(), CGE::Mesh::DRAW_ALL);
  
#endif
}

void Furniture::render2D() const {
#ifdef _CLIENT_
	if (rendered_)
		return;
  CGE::Gfx::DrawContext& ctx = *CGE::Engine::instance()->getDrawContext();
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
	int height = wrld.getMapSize().y;
	const CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
	float dx = (float)(screen.getWidth()/wrld.getMapSize().x);
	float dy = (float)(screen.getHeight()/height);
  ctx.setTexture(TextureManager::instance()->furnitureTex[id_-3000].get());
	switch(orientation_){	
		case TOP:
      forms->drawQuad(ctx, Vec2f(position_.x*dx, (height - position_.y - height_)*dy), Vec2f(dx*width_, dy*height_));
			break;
		case BOTTOM:
      forms->drawQuad(ctx, Vec2f((position_.x-width_+1)*dx, (height - (position_.y+1))*dy), Vec2f(dx*width_, dy*height_), 180);
			break;
		case RIGHT:
      forms->drawQuad(ctx, Vec2f((position_.x-height_+1)*dx, (height - (position_.y+width_))*dy), Vec2f(dx*width_, dy*height_), 270);
			break;
		case LEFT:
      forms->drawQuad(ctx, Vec2f((position_.x)*dx, (height - (position_.y+1))*dy), Vec2f(dx*width_, dy*height_), 90);
			break;
	}
  ctx.setTexture(nullptr);
#endif
}

void Furniture::update(){
	rendered_ = true;
}