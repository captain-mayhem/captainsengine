#ifndef FOLLOWCAM_H
#define FOLLOWCAM_H

#include <renderer/camera.h>

class GameObject;

class FollowCam : public CGE::Camera {
public:
  FollowCam();
  ~FollowCam();
  void setTarget(::GameObject* obj) {mTarget = obj;}
  //! moves behind the target, then sets the context's projection and view
  void activate(CGE::Gfx::DrawContext& ctx) override;
protected:
  ::GameObject* mTarget;
};

#endif
