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

#include <cge_export.h>

#include "../math/vector.h"
#include "../math/matrix.h"
#include "../math/frustum.h"

namespace CGE{

namespace Gfx{
class DrawContext;
}

class CGE_API Camera{
public:
  Camera();
  virtual ~Camera();
  void project(float angle, float ratio, float nearDist, float farDist);
  void lookAt(CGE::Vec3f eye, CGE::Vec3f look, CGE::Vec3f up);
  void walk(float units);
  void strafe(float units);
  void fly(float units);
  void pitch(float angle);
  void yaw(float angle);
  void roll(float angle);
  //! sets the projection and view of the old renderer
  virtual void activate();
  //! sets the context's projection (for its device) and view
  virtual void activate(Gfx::DrawContext& ctx);
  //! from world space to the camera's view space
  CGE::Matrix getViewMatrix() const;
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
