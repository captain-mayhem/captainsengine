
#include "camera.h"
#include "../renderer/renderer.h"
#include "../system/engine.h"

using namespace Math;
using namespace Graphics;

Camera::Camera(){

}

Camera::~Camera(){

}

void Camera::project(float angle, float ratio, float nearDist, float farDist){
  mFrustum.setProjection(angle,ratio,nearDist,farDist);
}

void Camera::lookAt(Math::Vec3f eye, Math::Vec3f look, Math::Vec3f up){
  mPosition = eye;
  mForward = look - mPosition;
  mForward.normalize();
  mSide = mForward.cross(up);
  mSide.normalize();
  mUp = mSide.cross(mForward);
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::walk(float units){
  mPosition = mPosition + mForward*units;
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::strafe(float units){
  mPosition = mPosition + mSide*units;
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::fly(float units){
  mPosition = mPosition + mUp*units;
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::pitch(float angle){
  Matrix rot = Matrix(Matrix::Rotation, mSide, angle);
  mForward = rot*mForward;
  mUp = rot*mUp;
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::yaw(float angle){
  Matrix rot = Matrix(Matrix::Rotation, mUp, angle);
  mForward = rot*mForward;
  mSide = rot*mSide;
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::roll(float angle){
  Matrix rot = Matrix(Matrix::Rotation, mForward, angle);
  mSide = rot*mSide;
  mUp = rot*mUp;
  mFrustum.setCoordSys(mPosition, mForward, mSide, mUp);
}

void Camera::activate(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->projection(mFrustum.getAngle(),mFrustum.getRatio(),mFrustum.getNearDist(),mFrustum.getFarDist());
  Matrix mat = Matrix(mSide, mUp, mForward*-1, Vec3f()/*eye*1*/)*Matrix(Matrix::Translation,mPosition*-1);
  rend->multiplyMatrix(mat);
}