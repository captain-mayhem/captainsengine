#include "arcball.h"

#include "../system/engine.h"
#include "../input/mouse.h"

const float ADJ_WIDTH = (1.0f / ((SCREENWIDTH - 1.0f)*0.5f));
const float ADJ_HEIGHT = (1.0f / ((SCREENHEIGHT - 1.0f)*0.5f));

using namespace CGE;

Arcball::Arcball(){
  startVec_ = Vector3D();
  endVec_ = Vector3D();
  isDragging_ = false;
  currRot_ = Matrix(Matrix::Identity);
  lastRot_ = Matrix(Matrix::Identity);
  transform_ = Matrix(Matrix::Identity);
  stepTransform_ = Matrix(Matrix::Identity);
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
    if (perp.length() > CGE::Epsilon){
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
  Vec2f tempVec = Vec2f(point.x, point.y);
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

void Arcball::update(const bool change, const bool reset, const Vector2D coords){
  if (reset){
    //Reset
    lastRot_ = Matrix(Matrix::Identity);
    currRot_ = Matrix(Matrix::Identity);
    transform_ = Matrix(Matrix::Rotation, currRot_);
  }
  if (!isDragging_){
    if (change){
      isDragging_ = true;
      lastRot_ = currRot_;
      click(coords);
    }
  }
  else{
    if (change){
      Quaternion quat;
      drag(coords, &quat);
      currRot_ = Matrix(quat);
      currRot_ = currRot_*lastRot_;
      stepTransform_ = currRot_*transform_.transpose();
      transform_ = Matrix(Matrix::Rotation, currRot_);
    }
    else{
      isDragging_ = false;
      stepTransform_ = Matrix(Matrix::Identity);
    }
  }
}
