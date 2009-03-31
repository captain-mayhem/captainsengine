
#include "CollisionBody.h"

#include "CollisionSpace.h"
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "../renderer/vertexbuffer.h"

using namespace CGE;

CollisionBody::CollisionBody(const Simulator& sim) : CollisionObject(), Body(sim){
}

CollisionBody::~CollisionBody(){

}

void CollisionBody::initCylinder(const CollisionSpace& space, float height, float radius, float mass){
  Body::initCylinder(height,radius,mass);
  mGeom = dCreateCylinder(space.mSpace, radius, height);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
  mPhysicsVB = Graphics::Forms::createCylinder(radius, height, 32);
}

void CollisionBody::initBox(const CollisionSpace& space, float width, float height, float depth, float mass){
  Body::initBox(width,height,depth,mass);
  mGeom = dCreateBox(space.mSpace, depth, height, width);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
  mPhysicsVB = Graphics::Forms::createBox(width, height, depth);
}

void CollisionBody::initSphere(const CollisionSpace& space, float radius, float mass){
  Body::initSphere(radius,mass);
  mGeom = dCreateSphere(space.mSpace, radius);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
}

void CollisionBody::render(const Graphics::Camera& cam){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->pushMatrix();
  Vec3f trans = getPosition();
  rend->translate(trans.x,trans.y,trans.z);
  rend->multiplyMatrix(getOrientation());
  mPhysicsVB->activate();
  if (dGeomGetClass(mGeom) == dCylinderClass){
    mPhysicsVB->draw(Graphics::VB_Tristrip, 0);
    mPhysicsVB->draw(Graphics::VB_Trifan, 1);
    mPhysicsVB->draw(Graphics::VB_Trifan, 2);
  }
  else if (dGeomGetClass(mGeom) == dBoxClass){
    mPhysicsVB->draw(Graphics::VB_Triangles, 0);
  }
  rend->popMatrix();
}
