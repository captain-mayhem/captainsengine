
#include "FollowCam.h"
#include "GameObject.h"

using namespace CGE;

FollowCam::FollowCam(){

}

FollowCam::~FollowCam(){

}

void FollowCam::activate(){
  CGE::Vec3f pos = mTarget->getPosition();
  pos = pos-mTarget->getDirection()*10;
  pos += Vec3f(0,1,0)*3;
  lookAt(pos,mTarget->getPosition(),Vec3f(0,1,0));
  CGE::Camera::activate();
}