#include "../system/engine.h"
#include "renderer.h"
#include "forms.h"

using namespace Graphics;

Forms::Forms(){
  quad_ = NULL;
  constructVBOs();
}

Forms::~Forms(){
  SAFE_DELETE(quad_);
}

void Forms::constructVBOs(){
  //generate textured quad
  Renderer* rend = System::Engine::instance()->getRenderer();
  quad_ = rend->createVertexBuffer();
  quad_->create(VB_POSITION | VB_TEXCOORD, 4, 0);
  quad_->lockVertexPointer();
  quad_->setPosition(3, Vector3D(0.5,0.5,0));
  quad_->setPosition(2, Vector3D(-0.5,0.5,0));
  quad_->setPosition(0, Vector3D(-0.5,-0.5,0));
  quad_->setPosition(1, Vector3D(0.5,-0.5,0));
  quad_->setTexCoord(3, Vec2f(1,0));
  quad_->setTexCoord(2, Vec2f(0,0));
  quad_->setTexCoord(0, Vec2f(0,1));
  quad_->setTexCoord(1, Vec2f(1,1));
  quad_->unlockVertexPointer();
  /*short* idx = quad_->lockIndexPointer();
  idx[0] = 0;
  idx[1] = 1;
  idx[2] = 2;
  idx[3] = 3;
  quad_->unlockIndexPointer();*/
}

void Forms::activateQuad(){
  quad_->activate();
}

void Forms::drawQuad(){
  quad_->draw(VB_Tristrip);
}
