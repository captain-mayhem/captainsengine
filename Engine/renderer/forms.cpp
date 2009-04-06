#define _USE_MATH_DEFINES
#include <math.h>
#include "../system/engine.h"
#include "renderer.h"
#include "forms.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace Graphics;
using namespace CGE;

Forms::Forms(){
  quad_ = NULL;
  cylinder_ = NULL;
  constructVBOs();
}

Forms::~Forms(){
  SAFE_DELETE(quad_);
}

void Forms::constructVBOs(){
  Renderer* rend = System::Engine::instance()->getRenderer();
  //generate textured quad
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
  cylinder_ = createCylinder(1,1,32);
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

VertexBuffer* Forms::createCylinder(float radius, float height, int cylinder_segments){
  Renderer* rend = System::Engine::instance()->getRenderer();
  VertexBuffer* cylinder = rend->createVertexBuffer();
  cylinder->create(VB_POSITION, 2*cylinder_segments+2, 3);
  cylinder->lockVertexPointer();
  float angle = 0;
  for (int i = 0; i < 2*cylinder_segments; i+=2){
    cylinder->setPosition(i, Vec3f(radius*cos(angle),-height/2, radius*sin(angle)));
    cylinder->setPosition(i+1, Vec3f(radius*cos(angle),height/2, radius*sin(angle)));
    angle += (float)(M_PI*2./cylinder_segments);
  }
  cylinder->setPosition(2*cylinder_segments, Vec3f(0.0f,-height/2,0.0f));
  cylinder->setPosition(2*cylinder_segments+1, Vec3f(0.0f,height/2,0.0f));
  cylinder->unlockVertexPointer();
  cylinder->createIndexBuffer(0, (cylinder_segments+1)*2);
  short* hull = cylinder->lockIndexPointer(0);
  for (int i = 0; i < (cylinder_segments+1)*2; ++i){
    hull[i] = i%(2*cylinder_segments);
  }
  cylinder->unlockIndexPointer(0);
  cylinder->createIndexBuffer(1, cylinder_segments+2);
  short* bottom = cylinder->lockIndexPointer(1);
  bottom[0] = 2*cylinder_segments;
  int count = 0;
  for (int i = 1; i <= cylinder_segments; ++i){
    bottom[i] = count;
    count+=2;
  }
  bottom[cylinder_segments+1] = 0;
  cylinder->unlockIndexPointer(1);
  cylinder->createIndexBuffer(2, cylinder_segments+2);
  short* top = cylinder->lockIndexPointer(2);
  top[0] = 2*cylinder_segments+1;
  count = 2*cylinder_segments-1;
  for (int i = 1; i <= cylinder_segments; ++i){
    top[i] = count;
    count-=2;
  }
  top[cylinder_segments+1] = 2*cylinder_segments-1;
  cylinder->unlockIndexPointer(2);
  return cylinder;
}

VertexBuffer* Forms::createBox(float width, float height, float depth){
  
   //1-----2//
  //      //
  //0----3// 6
  //|     |/ 
  //4---7//
  Renderer* rend = System::Engine::instance()->getRenderer();
  VertexBuffer* box = rend->createVertexBuffer();
  box->create(VB_POSITION, 8, 1);
  box->lockVertexPointer();
  box->setPosition(0, Vec3f(-width/2,height/2,depth/2));
  box->setPosition(1, Vec3f(-width/2,height/2,-depth/2));
  box->setPosition(2, Vec3f(width/2,height/2,-depth/2));
  box->setPosition(3, Vec3f(width/2,height/2,depth/2));
  box->setPosition(4, Vec3f(-width/2,-height/2,depth/2));
  box->setPosition(5, Vec3f(-width/2,-height/2,-depth/2));
  box->setPosition(6, Vec3f(width/2,-height/2,-depth/2));
  box->setPosition(7, Vec3f(width/2,-height/2,depth/2));
  box->unlockVertexPointer();
  box->createIndexBuffer(0, 36);
  short* idx = box->lockIndexPointer(0);
  //top
  idx[0] = 0; idx[1] = 3; idx[2] = 1;
  idx[3] = 1; idx[4] = 3; idx[5] = 2;
  //front
  idx[6] = 4; idx[7] = 7; idx[8] = 0;
  idx[9] = 0; idx[10] = 7; idx[11] = 3;
  //left
  idx[12] = 5; idx[13] = 4; idx[14] = 1;
  idx[15] = 1; idx[16] = 4; idx[17] = 0;
  //back
  idx[18] = 6; idx[19] = 5; idx[20] = 2;
  idx[21] = 2; idx[22] = 5; idx[23] = 1;
  //right
  idx[24] = 7; idx[25] = 6; idx[26] = 3;
  idx[27] = 3; idx[28] = 6; idx[29] = 2;
  //bottom
  idx[30] = 5; idx[31] = 6; idx[32] = 4;
  idx[33] = 4; idx[34] = 6; idx[35] = 7;
  box->unlockIndexPointer(0);
  return box;
}
