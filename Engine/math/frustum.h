//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-12-24                                                       |
// |------------------------------------------------------------------|
// | File: frustum.h                                                  |
//  ==================================================================

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"
#include "bbox.h"
#include "bsphere.h"

namespace Math{

class Frustum{
public:
  enum Result{
    INSIDE, 
    OUTSIDE, 
    INTERSECT,
  };
  Frustum();
  ~Frustum();
  void setProjection(float angle, float ratio, float nearDist, float farDist);
  void setCoordSys(const Vec3f& point, const Vec3f& forward, const Vec3f& side, const Vec3f& up);
  float getAngle() {return mAngle;}
  float getRatio() {return mRatio;}
  float getNearDist() {return mNearDist;}
  float getFarDist() {return mFarDist;}
  Result checkPoint(const Vec3f& p) const;
  Result checkSphere(const BSphere& sphere) const;
  Result checkBox(const BBox& box) const;
protected:
  enum PlaneDir{
    TOP=0,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK,
  };
  float mAngle;
  float mRatio;
  float mNearDist;
  float mNearWidth;
  float mNearHeight;
  float mFarDist;
  float mFarWidth;
  float mFarHeight;

  Plane mPlanes[6];
};

}

#endif
