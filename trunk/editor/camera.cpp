#include "camera.h"

EditCam::EditCam(){
  project(60, 4.0f / 3.0f, 0.01f, 1000.0f);
  camTrafo_ = CGE::Matrix(CGE::Matrix::Translation, CGE::Vec3f(0.0, 0.0, -10.0f));
  updatePosition();
}

void EditCam::updatePosition(){
  CGE::Matrix inv = camTrafo_.inverse();
  mPosition = inv*CGE::Vec3f();
  mForward = inv*CGE::Vec4f(0, 0, -1, 0);
  mSide = inv*CGE::Vec4f(1, 0, 0, 0);
  mUp = inv*CGE::Vec4f(0, 1, 0, 0);
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}