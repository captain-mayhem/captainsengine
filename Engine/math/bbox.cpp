#include <float.h>
#include "ray.h"
#include "bbox.h"

using namespace Math;

bool BBox::hit(const Ray& r) const{
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

  return (tmin < FLT_MAX && tmax > -1e-4);
}
