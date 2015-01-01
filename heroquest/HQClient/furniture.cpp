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

#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/gl.h>

#include "furniture.h"
#include "common.h"
#include "world.h"
#ifdef _CLIENT_
	#include "textureManager.h"
#include "renderer/forms.h"
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

  if (mModel){
    mModel->setupMaterial();
    mModel->render();
    mModel->resetMaterial();
  }
  
  /*TextureManager::instance()->furnitureTex[id_-3000]->activate();
	//glBindTexture(GL_TEXTURE_2D, tex.furnitureTex[id_]);
  glPushMatrix();
	
	switch(orientation_){
		case TOP:
			glTranslatef(position_.x*QUADSIZE + QUADSIZE/2 + QUADSIZE*(float)(width_-1)/2, 
									4, 
									position_.y*QUADSIZE + QUADSIZE/2 + QUADSIZE*(float)(height_-1)/2);
			glScalef(4.0f*width_, 4, 4.0f*height_);
			break;
		case BOTTOM:
			glTranslatef(position_.x*QUADSIZE + QUADSIZE/2 - QUADSIZE*(float)(width_-1)/2, 
									4, 
									position_.y*QUADSIZE + QUADSIZE/2 - QUADSIZE*(float)(height_-1)/2);
			glScalef(4.0f*width_, 4, 4.0f*height_);
			break;
		case RIGHT:
			glTranslatef(position_.x*QUADSIZE + QUADSIZE/2 - QUADSIZE*(float)(height_-1)/2, 
									4, 
									position_.y*QUADSIZE + QUADSIZE/2 + QUADSIZE*(float)(width_-1)/2);
			glScalef(4.0f*height_, 4, 4.0f*width_);
			break;
		case LEFT:
			glTranslatef(position_.x*QUADSIZE + QUADSIZE/2 + QUADSIZE*(float)(height_-1)/2, 
									4, 
									position_.y*QUADSIZE + QUADSIZE/2 - QUADSIZE*(float)(width_-1)/2);
			glScalef(4.0f*height_, 4, 4.0f*width_);
			break;
	}
	
	Forms::drawCube();
	glPopMatrix();*/
#endif
}

void Furniture::render2D() const {
#ifdef _CLIENT_
	if (rendered_)
		return;
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  CGE::Forms* forms = CGE::Engine::instance()->getForms();
	int height = wrld.getMapSize().y;
	float dx = (float)(SCREENWIDTH/wrld.getMapSize().x);
	float dy = (float)(SCREENHEIGHT/height);
  rend->enableTexturing(true);

  TextureManager::instance()->furnitureTex[id_-3000]->activate();
	switch(orientation_){	
		case TOP:
      forms->drawQuad(Vec2f(position_.x*dx, (height - position_.y - height_)*dy), Vec2f(dx*width_, dy*height_));
			break;
		case BOTTOM:
      forms->drawQuad(Vec2f((position_.x-width_+1)*dx, (height - (position_.y+1))*dy), Vec2f(dx*width_, dy*height_), 180);
			break;
		case RIGHT:
      forms->drawQuad(Vec2f((position_.x-height_+1)*dx, (height - (position_.y+width_))*dy), Vec2f(dx*width_, dy*height_), 270);
			break;
		case LEFT:
      forms->drawQuad(Vec2f((position_.x)*dx, (height - (position_.y+1))*dy), Vec2f(dx*width_, dy*height_), 90);
			break;
	}
#endif
}

void Furniture::update(){
	rendered_ = true;
}
