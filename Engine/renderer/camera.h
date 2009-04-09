//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-12-22                                                       |
// |------------------------------------------------------------------|
// | File: camera.h                                                   |
//  ==================================================================

#ifndef CGE_CAMERA_H
#define CGE_CAMERA_H

#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/frustum.h"

namespace CGE{

class Camera{
public:
  Camera();
  ~Camera();
  void project(float angle, float ratio, float nearDist, float farDist);
  void lookAt(CGE::Vec3f eye, CGE::Vec3f look, CGE::Vec3f up);
  void walk(float units);
  void strafe(float units);
  void fly(float units);
  void pitch(float angle);
  void yaw(float angle);
  void roll(float angle);
  void activate();
  const CGE::Frustum& getFrustum() const {return mFrustum;}
  const CGE::Vec3f& getPosition() const {return mPosition;}
protected:
  CGE::Vec3f mPosition;
  CGE::Vec3f mForward;
  CGE::Vec3f mSide;
  CGE::Vec3f mUp;
  CGE::Frustum mFrustum;
};

}

#endif
