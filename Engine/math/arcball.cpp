#include "arcball.h"

#include "../system/engine.h"

const float ADJ_WIDTH = (1.0f / ((SCREENWIDTH - 1.0f)*0.5f));
const float ADJ_HEIGHT = (1.0f / ((SCREENHEIGHT - 1.0f)*0.5f));

using namespace Math;

Arcball::Arcball(){
}

Arcball::~Arcball(){
}

void Arcball::click(const Vector2D& point){
  mapToSphere(point, startVec_);
}

void Arcball::drag(const Vector2D& point, Quaternion* rot){
  mapToSphere(point, endVec_);
  if (rot){
    Vector3D perp = startVec_.cross(endVec_);
    if (perp.length() > Math::Epsilon){
      rot->x = perp.x;
      rot->y = perp.y;
      rot->z = perp.z;
      rot->w = startVec_.dot(endVec_);
    }
    else{
      *rot = Quaternion();
    }
  }
}

void Arcball::mapToSphere(const Vector2D& point, Vector3D& newVec) const{
  Vec2f tempVec = Vec2f(newVec.x, newVec.y);
  //scale to [-1..1]
  tempVec.x = (tempVec.x * ADJ_WIDTH) - 1.0f;
  tempVec.y = 1.0f - (tempVec.y * ADJ_HEIGHT);
  float length = tempVec.lengthSquared();
  //outside of the sphere?
  if (length > 1.0f){
    float norm = sqrt(length);
    newVec.x = tempVec.x*norm;
    newVec.y = tempVec.y*norm;
    newVec.z = 0.0f;
  }
  else{
    newVec.x = tempVec.x;
    newVec.y = tempVec.y;
    newVec.z = sqrt(1.0f-length);
  }
}
