
#include "camera.h"
#include "../system/engine.h"
#include "drawcontext.h"
#include "renderdevice.h"

using namespace CGE;

Camera::Camera(){

}

Camera::~Camera(){

}

void Camera::project(float angle, float ratio, float nearDist, float farDist){
  mFrustum.setProjection(angle,ratio,nearDist,farDist);
}

void Camera::lookAt(CGE::Vec3f eye, CGE::Vec3f look, CGE::Vec3f up){
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

void Camera::activate(Gfx::DrawContext& ctx){
  ctx.setProjection(ctx.getDevice().perspective(mFrustum.getAngle(), mFrustum.getRatio(), mFrustum.getNearDist(), mFrustum.getFarDist()));
  ctx.setView(getViewMatrix());
}

Matrix Camera::getViewMatrix() const{
  return Matrix(mSide, mUp, mForward*-1, Vec3f())*Matrix(Matrix::Translation, mPosition*-1);
}

