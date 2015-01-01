#define _USE_MATH_DEFINES
#include <math.h>
#include "../system/engine.h"
#include "renderer.h"
#include "forms.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace CGE;

Forms::Forms(){
  quad_ = NULL;
  cylinder_ = NULL;
  for (int i = 0; i < 3; ++i){
    mCylinderInds[i] = NULL;
  }
  constructVBOs();
}

Forms::~Forms(){
  SAFE_DELETE(quad_);
  SAFE_DELETE(cylinder_);
  for (int i = 0; i < 3; ++i){
    SAFE_DELETE(mCylinderInds[i]);
  }
}

void Forms::constructVBOs(){
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  //generate textured quad
  quad_ = rend->createVertexBuffer();
  if (quad_ == NULL)
    return;
  quad_->create(VB_POSITION | VB_TEXCOORD, 4);
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
  cylinder_ = createCylinder(1,1,32,mCylinderInds);
}

void Forms::activateQuad(){
  quad_->activate();
}

void Forms::drawQuad(){
  quad_->draw(VB_Tristrip, 0);
}

void Forms::drawQuad(const Vector2D& position, const Vector2D& size, float rotation) const {
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->pushMatrix();
  Vec2f offset = size/2.0f;
  if (rotation != 0){
    Matrix rot = Matrix(Matrix::Rotation, Vec3f(0, 0, -1), rotation/180*(float)M_PI);
    Vec3f rotoff = rot * Vec3f(offset.x, offset.y, 0);
    offset = Vec2f(abs(rotoff.x), abs(rotoff.y));
  }
  rend->translate(position.x+offset.x, position.y+offset.y, 0);
  if (rotation != 0)
    rend->rotate(rotation, 0, 0, -1);
  rend->scale(size.x, size.y, 1);
  quad_->draw(VB_Tristrip, 0);
  rend->popMatrix();
}

void Forms::activateCylinder(){
  cylinder_->activate();
}

void Forms::drawCylinder(){
  cylinder_->draw(VB_Tristrip, mCylinderInds[0]);
  cylinder_->draw(VB_Trifan, mCylinderInds[1]);
  cylinder_->draw(VB_Trifan, mCylinderInds[2]);
}

VertexBuffer* Forms::createCylinder(float radius, float height, int cylinder_segments, IndexBuffer* indices[3]){
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  VertexBuffer* cylinder = rend->createVertexBuffer();
  cylinder->create(VB_POSITION | VB_NORMAL, 4*cylinder_segments+2);
  cylinder->lockVertexPointer();
  float angle = 0;
  for (int i = 0; i < 2*cylinder_segments; i+=2){
    //the hull
    cylinder->setPosition(i, Vec3f(radius*cos(angle),-height/2, radius*sin(angle)));
    cylinder->setNormal(i, Vec3f(cos(angle),0,sin(angle)));
    cylinder->setPosition(i+1, Vec3f(radius*cos(angle),height/2, radius*sin(angle)));
    cylinder->setNormal(i+1, Vec3f(cos(angle),0,sin(angle)));
    //the caps
    cylinder->setPosition(2*cylinder_segments+i, Vec3f(radius*cos(angle),-height/2, radius*sin(angle)));
    cylinder->setNormal(2*cylinder_segments+i, Vec3f(0,-1,0));
    cylinder->setPosition(2*cylinder_segments+i+1, Vec3f(radius*cos(angle),height/2, radius*sin(angle)));
    cylinder->setNormal(2*cylinder_segments+i+1, Vec3f(0,1,0));
    angle += (float)(M_PI*2./cylinder_segments);
  }
  cylinder->setPosition(4*cylinder_segments, Vec3f(0.0f,-height/2,0.0f));
  cylinder->setNormal(4*cylinder_segments, Vec3f(0,-1,0));
  cylinder->setPosition(4*cylinder_segments+1, Vec3f(0.0f,height/2,0.0f));
  cylinder->setNormal(4*cylinder_segments+1, Vec3f(0,1,0));
  cylinder->unlockVertexPointer();

  indices[0] = rend->createIndexBuffer(IndexBuffer::IB_USHORT,(cylinder_segments+1)*2);
  short* hull = (short*)indices[0]->lockIndexPointer();
  for (int i = 0; i < (cylinder_segments+1)*2; ++i){
    hull[i] = i%(2*cylinder_segments);
  }
  indices[0]->unlockIndexPointer();
  indices[1] = rend->createIndexBuffer(IndexBuffer::IB_USHORT,cylinder_segments+2);
  short* bottom = (short*)indices[1]->lockIndexPointer();
  bottom[0] = 4*cylinder_segments;
  int count = 0;
  for (int i = 1; i <= cylinder_segments; ++i){
    bottom[i] = 2*cylinder_segments+count;
    count+=2;
  }
  bottom[cylinder_segments+1] = 2*cylinder_segments;
  indices[1]->unlockIndexPointer();
  indices[2] = rend->createIndexBuffer(IndexBuffer::IB_USHORT,cylinder_segments+2);
  short* top = (short*)indices[2]->lockIndexPointer();
  top[0] = 4*cylinder_segments+1;
  count = 4*cylinder_segments-1;
  for (int i = 1; i <= cylinder_segments; ++i){
    top[i] = count;
    count-=2;
  }
  top[cylinder_segments+1] = 4*cylinder_segments-1;
  indices[2]->unlockIndexPointer();
  return cylinder;
}

VertexBuffer* Forms::createBox(float width, float height, float depth, IndexBuffer** indices){
  
   //1-----2//
  //      //
  //0----3// 6
  //|     |/ 
  //4---7//
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  VertexBuffer* box = rend->createVertexBuffer();
  box->create(VB_POSITION | VB_NORMAL, 24);
  box->lockVertexPointer();
  box->setPosition(0, Vec3f(-width/2,height/2,depth/2));
  box->setNormal(0, Vec3f(-1,0,0));
  box->setPosition(8, Vec3f(-width/2,height/2,depth/2));
  box->setNormal(8, Vec3f(0,1,0));
  box->setPosition(16, Vec3f(-width/2,height/2,depth/2));
  box->setNormal(16, Vec3f(0,0,1));

  box->setPosition(1, Vec3f(-width/2,height/2,-depth/2));
  box->setNormal(1, Vec3f(-1,0,0));
  box->setPosition(9, Vec3f(-width/2,height/2,-depth/2));
  box->setNormal(9, Vec3f(0,1,0));
  box->setPosition(17, Vec3f(-width/2,height/2,-depth/2));
  box->setNormal(17, Vec3f(0,0,-1));

  box->setPosition(2, Vec3f(width/2,height/2,-depth/2));
  box->setNormal(2, Vec3f(1,0,0));
  box->setPosition(10, Vec3f(width/2,height/2,-depth/2));
  box->setNormal(10, Vec3f(0,1,0));
  box->setPosition(18, Vec3f(width/2,height/2,-depth/2));
  box->setNormal(18, Vec3f(0,0,-1));

  box->setPosition(3, Vec3f(width/2,height/2,depth/2));
  box->setNormal(3, Vec3f(1,0,0));
  box->setPosition(11, Vec3f(width/2,height/2,depth/2));
  box->setNormal(11, Vec3f(0,1,0));
  box->setPosition(19, Vec3f(width/2,height/2,depth/2));
  box->setNormal(19, Vec3f(0,0,1));

  box->setPosition(4, Vec3f(-width/2,-height/2,depth/2));
  box->setNormal(4, Vec3f(-1,0,0));
  box->setPosition(12, Vec3f(-width/2,-height/2,depth/2));
  box->setNormal(12, Vec3f(0,-1,0));
  box->setPosition(20, Vec3f(-width/2,-height/2,depth/2));
  box->setNormal(20, Vec3f(0,0,1));

  box->setPosition(5, Vec3f(-width/2,-height/2,-depth/2));
  box->setNormal(5, Vec3f(-1,0,0));
  box->setPosition(13, Vec3f(-width/2,-height/2,-depth/2));
  box->setNormal(13, Vec3f(0,-1,0));
  box->setPosition(21, Vec3f(-width/2,-height/2,-depth/2));
  box->setNormal(21, Vec3f(0,0,-1));

  box->setPosition(6, Vec3f(width/2,-height/2,-depth/2));
  box->setNormal(6, Vec3f(1,0,0));
  box->setPosition(14, Vec3f(width/2,-height/2,-depth/2));
  box->setNormal(14, Vec3f(0,-1,0));
  box->setPosition(22, Vec3f(width/2,-height/2,-depth/2));
  box->setNormal(22, Vec3f(0,0,-1));

  box->setPosition(7, Vec3f(width/2,-height/2,depth/2));
  box->setNormal(7, Vec3f(1,0,0));
  box->setPosition(15, Vec3f(width/2,-height/2,depth/2));
  box->setNormal(15, Vec3f(0,-1,0));
  box->setPosition(23, Vec3f(width/2,-height/2,depth/2));
  box->setNormal(23, Vec3f(0,0,1));
  box->unlockVertexPointer();
  
  indices[0] = rend->createIndexBuffer(IndexBuffer::IB_USHORT, 36);
  short* idx = (short*)indices[0]->lockIndexPointer();
  //top
  idx[0] = 8; idx[1] = 11; idx[2] = 9;
  idx[3] = 9; idx[4] = 11; idx[5] = 10;
  //front
  idx[6] = 20; idx[7] = 23; idx[8] = 16;
  idx[9] = 16; idx[10] = 23; idx[11] = 19;
  //left
  idx[12] = 5; idx[13] = 4; idx[14] = 1;
  idx[15] = 1; idx[16] = 4; idx[17] = 0;
  //back
  idx[18] = 22; idx[19] = 21; idx[20] = 18;
  idx[21] = 18; idx[22] = 21; idx[23] = 17;
  //right
  idx[24] = 7; idx[25] = 6; idx[26] = 3;
  idx[27] = 3; idx[28] = 6; idx[29] = 2;
  //bottom
  idx[30] = 13; idx[31] = 14; idx[32] = 12;
  idx[33] = 12; idx[34] = 14; idx[35] = 15;
  indices[0]->unlockIndexPointer();
  return box;
}
