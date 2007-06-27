
#include "bsphere.h"
#include "ray.h"
#include "matrix.h"

using namespace Math;

BSphere::BSphere(){
  center_ = Vector3D();
  radius_ = 0.0f;
}

BSphere::BSphere(const Vector3D& center, const float radius){
  center_ = center;
  radius_ = radius;
}

BSphere::~BSphere(){
}

bool BSphere::hit(const Ray& r) const{
  Vector3D v = r.getOrigin() - center_;
  float b = 2.0f*r.getDirection().dot(v);
  float c = v.dot(v) - radius_*radius_;
  float discriminant = b*b - 4.0f*c;

  //no solution
  if (discriminant < 0.0)
    return false;

  discriminant = sqrtf(discriminant);

  float s0 = (-b + discriminant)/2.0f;
  float s1 = (-b - discriminant)/2.0f;

  if (s0 >= 0.0f || s1 >= 0.0f)
    return true;

  return false;
}

void BSphere::transform(const Matrix& mat){
  center_ = mat*center_;
}

BoundingObject* BSphere::copy(){
  return new BSphere(center_, radius_);
}

