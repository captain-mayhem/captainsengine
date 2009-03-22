#define _USE_MATH_DEFINES
#include "../system/engine.h"
#include "renderer.h"
#include "forms.h"

using namespace Graphics;

static const int cylinder_segments = 32;

Forms::Forms(){
  quad_ = NULL;
  cylinder_ = NULL;
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
  
  //generate cylinder
  cylinder_ = rend->createVertexBuffer();
  cylinder_->create(VB_POSITION, 2*cylinder_segments+2, 3);
  cylinder_->lockVertexPointer();
  float angle = 0;
  for (int i = 0; i < 2*cylinder_segments; i+=2){
    cylinder_->setPosition(i, Vec3f(cos(angle),-0.5, sin(angle)));
    cylinder_->setPosition(i+1, Vec3f(cos(angle),0.5, sin(angle)));
    angle += (float)(M_PI*2./cylinder_segments);
  }
  cylinder_->setPosition(2*cylinder_segments, Vec3f(0.0f,-0.5f,0.0f));
  cylinder_->setPosition(2*cylinder_segments+1, Vec3f(0.0f,0.5f,0.0f));
  cylinder_->unlockVertexPointer();
  cylinder_->createIndexBuffer(0, (cylinder_segments+1)*2);
  short* hull = cylinder_->lockIndexPointer(0);
  for (int i = 0; i < (cylinder_segments+1)*2; ++i){
    hull[i] = i%(2*cylinder_segments);
  }
  cylinder_->unlockIndexPointer(0);
  cylinder_->createIndexBuffer(1, cylinder_segments+2);
  short* bottom = cylinder_->lockIndexPointer(1);
  bottom[0] = 2*cylinder_segments;
  int count = 0;
  for (int i = 1; i <= cylinder_segments; ++i){
    bottom[i] = count;
    count+=2;
  }
  bottom[cylinder_segments+1] = 0;
  cylinder_->unlockIndexPointer(1);
  cylinder_->createIndexBuffer(2, cylinder_segments+2);
  short* top = cylinder_->lockIndexPointer(2);
  top[0] = 2*cylinder_segments+1;
  count = 2*cylinder_segments-1;
  for (int i = 1; i <= cylinder_segments; ++i){
    top[i] = count;
    count-=2;
  }
  top[cylinder_segments+1] = 2*cylinder_segments-1;
  cylinder_->unlockIndexPointer(2);
}

void Forms::activateQuad(){
  quad_->activate();
}

void Forms::drawQuad(){
  quad_->draw(VB_Tristrip, 0);
}

void Forms::drawQuad(const Vector2D& position, const Vector2D& size) const {
  Renderer* rend = System::Engine::instance()->getRenderer();
  rend->pushMatrix();
  rend->translate(position.x+size.x/2.0f, position.y+size.y/2.0f, 0);
  rend->scale(size.x, size.y, 1);
  quad_->draw(VB_Tristrip, 0);
  rend->popMatrix();
}

void Forms::activateCylinder(){
  cylinder_->activate();
}

void Forms::drawCylinder(){
  cylinder_->draw(VB_Tristrip, 0);
  cylinder_->draw(VB_Trifan, 1);
  cylinder_->draw(VB_Trifan, 2);
}
