#include "plane.h"

using namespace CGE;

Plane::Plane() : mNormal(), mPoint(), mD(0){
}

Plane::Plane(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3){
  setPoints(p1,p2,p3);
}

Plane::~Plane(){

}

void Plane::setPoints(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3){
  Vec3f v1 = p1-p2;
  Vec3f v2 = p3-p2;
  mNormal = v2.cross(v1);
  mNormal.normalize();
  mPoint = p2;
  mD = -mNormal.dot(mPoint);
}

float Plane::distance(const Vec3f& p) const{
  return mD + mNormal.dot(p);
}
