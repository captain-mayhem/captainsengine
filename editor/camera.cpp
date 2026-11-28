#include "camera.h"

EditCam::EditCam(){
  project(60, 4.0f / 3.0f, 0.01f, 1000.0f);
  //it looks at the origin
  mOrbitDistance = 10.0f;
  camTrafo_ = CGE::Matrix(CGE::Matrix::Translation, CGE::Vec3f(0.0, 0.0, -mOrbitDistance));
  updatePosition();
}

void EditCam::orbit(const CGE::Matrix& rot){
  CGE::Matrix toPivot(CGE::Matrix::Translation, CGE::Vec3f(0, 0, mOrbitDistance));
  CGE::Matrix fromPivot(CGE::Matrix::Translation, CGE::Vec3f(0, 0, -mOrbitDistance));
  multCamTrafo(fromPivot*rot*toPivot);
}

void EditCam::moveForward(float units){
  multCamTrafo(CGE::Matrix(CGE::Matrix::Translation, CGE::Vec3f(0, 0, units)));
  //the point it orbits stays, unless the camera passes it
  mOrbitDistance -= units;
  if (mOrbitDistance < 0.01f)
    mOrbitDistance = 0.01f;
}

void EditCam::lookAtFrom(const CGE::Vec3f& center, float distance){
  CGE::Vec3f eye = center - mForward*distance;
  setCamTrafo(CGE::Matrix(mSide, mUp, mForward*-1, CGE::Vec3f())*CGE::Matrix(CGE::Matrix::Translation, eye*-1));
  mOrbitDistance = distance;
  //near and far for the model's size
  const CGE::Frustum& frustum = getFrustum();
  float nearDist = distance/200.0f;
  float farDist = distance*100.0f;
  if (farDist < 100.0f)
    farDist = 100.0f;
  project(frustum.getAngle(), frustum.getRatio(), nearDist, farDist);
}

void EditCam::updatePosition(){
  CGE::Matrix inv = camTrafo_.inverse();
  mPosition = inv*CGE::Vec3f();
  mForward = inv*CGE::Vec4f(0, 0, -1, 0);
  mSide = inv*CGE::Vec4f(1, 0, 0, 0);
  mUp = inv*CGE::Vec4f(0, 1, 0, 0);
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}
