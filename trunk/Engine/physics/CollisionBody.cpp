
#include "CollisionBody.h"

#include "CollisionSpace.h"
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "../renderer/vertexbuffer.h"

using namespace CGE;

CollisionBody::CollisionBody(const Simulator& sim) : CollisionObject(), Body(sim){
  mPhysicsVB = NULL;
  for (int i = 0; i < 3; ++i){
    mPhysicsInds[i] = NULL;
  }
}

CollisionBody::~CollisionBody(){
  delete mPhysicsVB;
  for (int i = 0; i < 3; ++i){
    delete mPhysicsInds[i];
  }
}

CollisionBase::Type CollisionBody::getType(){
  int type = dGeomGetClass(mGeom);
  if (type == dCylinderClass)
    return Cylinder;
  if (type == dBoxClass)
    return Box;
  if (type == dSphereClass)
    return Sphere;
  return Undefined;
}

void CollisionBody::initCylinder(const CollisionSpace& space, float height, float radius, float mass){
  Body::initCylinder(height,radius,mass);
  mGeom = dCreateCylinder(space.mSpace, radius, height);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
  mPhysicsVB = CGE::Forms::createCylinder(radius, height, 32, mPhysicsInds);
}

void CollisionBody::initBox(const CollisionSpace& space, float width, float height, float depth, float mass){
  Body::initBox(width,height,depth,mass);
  mGeom = dCreateBox(space.mSpace, depth, height, width);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
  mPhysicsVB = CGE::Forms::createBox(width, height, depth, mPhysicsInds);
}

void CollisionBody::initSphere(const CollisionSpace& space, float radius, float mass){
  Body::initSphere(radius,mass);
  mGeom = dCreateSphere(space.mSpace, radius);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
}

void CollisionBody::render(const CGE::Camera& cam){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->pushMatrix();
  Vec3f trans = getPosition();
  rend->translate(trans.x,trans.y,trans.z);
  rend->multiplyMatrix(getOrientation());
  mPhysicsVB->activate();
  if (dGeomGetClass(mGeom) == dCylinderClass){
    mPhysicsVB->draw(CGE::VB_Tristrip, mPhysicsInds[0]);
    mPhysicsVB->draw(CGE::VB_Trifan, mPhysicsInds[1]);
    mPhysicsVB->draw(CGE::VB_Trifan, mPhysicsInds[2]);
  }
  else if (dGeomGetClass(mGeom) == dBoxClass){
    mPhysicsVB->draw(CGE::VB_Triangles, mPhysicsInds[0]);
  }
  rend->popMatrix();
}
