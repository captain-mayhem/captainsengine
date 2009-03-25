#include <float.h>
#include "ray.h"
#include "bbox.h"
#include "matrix.h"

using namespace CGE;
using std::min;

BBox::BBox(){
  min_ = Vector3D(FLT_MAX, FLT_MAX, FLT_MAX);
  max_ = Vector3D(FLT_MIN, FLT_MIN, FLT_MIN);
}

BBox::BBox(const Vector3D& min, const Vector3D& max){
  min_ = min;
  max_ = max;
}

BBox::~BBox(){
}

float BBox::hit(const Ray& r) const{
  float tmin, tmax;
  float tymin, tymax;
  float tzmin, tzmax;
  if (r.getDirection().x >= 0){
    tmin = (min_.x - r.getOrigin().x)/r.getDirection().x;
    tmax = (max_.x - r.getOrigin().x)/r.getDirection().x;
  }
  else{
    tmin = (max_.x - r.getOrigin().x)/r.getDirection().x;
    tmax = (min_.x - r.getOrigin().x)/r.getDirection().x;
  }
  
  if (r.getDirection().y >= 0){
    tymin = (min_.y - r.getOrigin().y)/r.getDirection().y;
    tymax = (max_.y - r.getOrigin().y)/r.getDirection().y;
  }
  else{
    tymin = (max_.y - r.getOrigin().y)/r.getDirection().y;
    tymax = (min_.y - r.getOrigin().y)/r.getDirection().y;
  }
  if (tmin > tymax || tymin > tmax)
    return false;
  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;

  if (r.getDirection().z >= 0){
    tzmin = (min_.z - r.getOrigin().z)/r.getDirection().z;
    tzmax = (max_.z - r.getOrigin().z)/r.getDirection().z;
  }
  else{
    tzmin = (max_.z - r.getOrigin().z)/r.getDirection().z;
    tzmax = (min_.z - r.getOrigin().z)/r.getDirection().z;
  }
  if (tmin > tzmax || tzmin > tmax)
    return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  if (tmin < FLT_MAX && tmax > -1e-4){
    return min(tmin, tmax);
  }
  return FLT_MAX;//(tmin < FLT_MAX && tmax > -1e-4);
}

void BBox::transform(const Matrix& mat){
  min_ = mat*min_;
  max_ = mat*max_;
}

BoundingObject* BBox::copy(){
  return new BBox(min_, max_);
}

Vec3f BBox::getNearestCorner(const Vec3f& normal) const{
  Vec3f res = min_;
  if (normal.x > 0)
    res.x = max_.x;
  if (normal.y > 0)
    res.y = max_.y;
  if (normal.z > 0)
    res.z = max_.z;
  return res;
}

Vec3f BBox::getFarestCorner(const Vec3f& normal) const{
  Vec3f res = min_;
  if (normal.x < 0)
    res.x = max_.x;
  if (normal.y < 0)
    res.y = max_.y;
  if (normal.z < 0)
    res.z = max_.z;
  return res;
}

