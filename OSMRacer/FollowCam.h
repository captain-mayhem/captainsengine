#ifndef FOLLOWCAM_H
#define FOLLOWCAM_H

#include <renderer/camera.h>

class GameObject;

class FollowCam : public CGE::Camera {
public:
  FollowCam();
  ~FollowCam();
  void setTarget(::GameObject* obj) {mTarget = obj;}
  virtual void activate();
protected:
  ::GameObject* mTarget;
};

#endif
