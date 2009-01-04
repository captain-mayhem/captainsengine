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

namespace Graphics{

class Camera{
public:
  Camera();
  ~Camera();
  void project(float angle, float ratio, float nearDist, float farDist);
  void lookAt(Math::Vec3f eye, Math::Vec3f look, Math::Vec3f up);
  void walk(float units);
  void strafe(float units);
  void fly(float units);
  void pitch(float angle);
  void yaw(float angle);
  void roll(float angle);
  void activate();
  const Math::Frustum& getFrustum() const {return mFrustum;}
protected:
  Math::Vec3f mPosition;
  Math::Vec3f mForward;
  Math::Vec3f mSide;
  Math::Vec3f mUp;
  Math::Frustum mFrustum;
};

}

#endif
