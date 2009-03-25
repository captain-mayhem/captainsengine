#include "ray.h"

#include "matrix.h"

using namespace CGE;

Ray::Ray(){
  origin_ = Vector3D();
  direction_ = Vector3D();
}
  
Ray::Ray(const Vector3D& orig, const Vector3D dir){
  origin_ = orig;
  direction_ = dir;
}

//! generate a picking ray
void Ray::buildPickingRay(float x, float y, float width, float height, float nww, float nwh, float nwd){
  float px = (((2.0f*x)/width)-1.0f)/nww;
  float py = (((-2.0f*y)/height)+1.0f)/nwh;
  origin_ = Vector3D();
  direction_ = Vector3D(px,py,1.0/nwd);
}

//! transform a ray
void Ray::transform(const Matrix& mat){
  //origin_ = mat*origin_;
  //direction_ = mat/*.inverse().transpose()*/*direction_;
  //direction_.normalize();
  Vector3D tmp = origin_+direction_;
  tmp = mat*tmp;
  origin_ = mat*origin_;
  direction_ = (tmp-origin_).normalized();
}

