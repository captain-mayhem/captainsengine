
#include "frustum.h"

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace CGE;

Frustum::Frustum(){

}

Frustum::~Frustum(){

}

void Frustum::setProjection(float angle, float ratio, float nearDist, float farDist){
  mAngle = angle;
  mRatio = ratio;
  mNearDist = nearDist;
  mFarDist = farDist;
  float tang = (float)tan(angle*M_PI/180.0*0.5);
  mNearHeight = mNearDist*tang;
  mNearWidth = mNearHeight*ratio;
  mFarHeight = mFarDist*tang;
  mFarWidth = mFarHeight*ratio;
}

void Frustum::setCoordSys(const Vec3f& point, const Vec3f& forward, const Vec3f& side, const Vec3f& up){
  Vec3f nearCenter = point + forward*mNearDist;
  Vec3f farCenter = point+ forward*mFarDist;

  Vec3f nearTopLeft = nearCenter + up*mNearHeight - side*mNearWidth;
  Vec3f nearTopRight = nearCenter + up*mNearHeight + side*mNearWidth;
  Vec3f nearBottomLeft = nearCenter - up*mNearHeight - side*mNearWidth;
  Vec3f nearBottomRight = nearCenter - up*mNearHeight + side*mNearWidth;

  Vec3f farTopLeft = farCenter + up*mFarHeight - side*mFarWidth;
  Vec3f farTopRight = farCenter + up*mFarHeight + side*mFarWidth;
  Vec3f farBottomLeft = farCenter - up*mFarHeight - side*mFarWidth;
  Vec3f farBottomRight = farCenter - up*mFarHeight + side*mFarWidth;

  mPlanes[TOP].setPoints(nearTopRight,nearTopLeft,farTopLeft);
  mPlanes[BOTTOM].setPoints(nearBottomLeft,nearBottomRight,farBottomRight);
  mPlanes[LEFT].setPoints(nearTopLeft,nearBottomLeft,farBottomLeft);
  mPlanes[RIGHT].setPoints(nearBottomRight,nearTopRight,farBottomRight);
  mPlanes[FRONT].setPoints(nearTopLeft,nearTopRight,nearBottomRight);
  mPlanes[BACK].setPoints(farTopRight,farTopLeft,farBottomLeft);
}

Frustum::Result Frustum::checkPoint(const Vec3f& p) const{
  for(int i = 0; i < 6; ++i){
    if (mPlanes[i].distance(p) < 0){
      return OUTSIDE;
    }
  }
  return INSIDE;
}

Frustum::Result Frustum::checkSphere(const BSphere& sphere) const{
  Result res = INSIDE;
  for(int i = 0; i < 6; ++i){
    float distance = mPlanes[i].distance(sphere.getCenter());
    if (distance < -sphere.getRadius()){
      return OUTSIDE;
    }
    else if (distance < sphere.getRadius()){
      res = INTERSECT;
    }
  }
  return res;
}

Frustum::Result Frustum::checkBox(const BBox& box) const{
  Result res = INSIDE;
  for(int i = 0; i < 6; ++i){
    if (mPlanes[i].distance(box.getNearestCorner(mPlanes[i].getNormal())) < 0){
      return OUTSIDE;
    }
    else if (mPlanes[i].distance(box.getFarestCorner(mPlanes[i].getNormal())) < 0){
      res = INTERSECT;
    }
  }
  return res;
}

