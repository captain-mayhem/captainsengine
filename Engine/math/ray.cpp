#include "ray.h"

using namespace Math;

Ray::Ray(){
  origin_ = Vector3D();
  direction_ = Vector3D();
}
  
Ray::Ray(const Vector3D& orig, const Vector3D dir){
  origin_ = orig;
  direction_ = dir;
}

