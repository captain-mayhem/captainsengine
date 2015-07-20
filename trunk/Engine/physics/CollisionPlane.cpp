
#include "CollisionPlane.h"

using namespace CGE;

CollisionPlane::CollisionPlane(const Vec3f& normal, float distance){
  mGeom = dCreatePlane(0,normal.x,normal.y,normal.z,distance);
  dGeomSetData(mGeom, this);
}

CollisionPlane::~CollisionPlane(){

}

void CollisionPlane::render(const CGE::Camera& cam){

}
