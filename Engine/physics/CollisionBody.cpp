
#include "CollisionBody.h"

#include "CollisionSpace.h"
#include "../system/engine.h"
#include "../renderer/drawcontext.h"
#include "../math/matrixstack.h"

using namespace CGE;

CollisionBody::CollisionBody(const Simulator& sim) : CollisionObject(), Body(sim){
}

CollisionBody::~CollisionBody(){
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
  if (Gfx::RenderDevice* device = Engine::instance()->getRenderDevice())
    mShape = Forms::createCylinder(*device, radius, height, 32);
}

void CollisionBody::initBox(const CollisionSpace& space, float width, float height, float depth, float mass){
  Body::initBox(width,height,depth,mass);
  mGeom = dCreateBox(space.mSpace, depth, height, width);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
  if (Gfx::RenderDevice* device = Engine::instance()->getRenderDevice())
    mShape = Forms::createBox(*device, width, height, depth);
}

void CollisionBody::initSphere(const CollisionSpace& space, float radius, float mass){
  Body::initSphere(radius,mass);
  mGeom = dCreateSphere(space.mSpace, radius);
  dGeomSetBody(mGeom, mBody);
  dGeomSetData(mGeom, this);
}

void CollisionBody::render(Gfx::DrawContext& ctx, const CGE::Camera& cam){
  if (!mShape)
    return;
  MatrixStack& model = ctx.model();
  model.push();
  Vec3f trans = getPosition();
  model.translate(trans.x,trans.y,trans.z);
  model.multiply(getOrientation());
  Forms::drawShape(ctx, *mShape);
  model.pop();
}
