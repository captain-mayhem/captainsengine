#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <renderer/camera.h>

class EditCam : public CGE::Camera{
public:
  EditCam();
  //! set translation of the camera
  void setCamTrafo(const CGE::Matrix& mat) { camTrafo_ = mat; updatePosition(); }
  //! set transformation of the camera
  void multCamTrafo(const CGE::Matrix& mat) { camTrafo_ = mat * camTrafo_; updatePosition(); }
  //! get view matrix
  CGE::Matrix getViewMat() { return camTrafo_; }
  //! turns the camera around the point it orbits (rot: a rotation in view space)
  void orbit(const CGE::Matrix& rot);
  //! moves the camera forward (backward for negative units), towards the point it orbits
  void moveForward(float units);
  //! looks at center from distance, from the direction the camera looks now; it orbits center then
  void lookAtFrom(const CGE::Vec3f& center, float distance);
  //! the distance in front of the camera of the point it orbits
  float getOrbitDistance() const { return mOrbitDistance; }
private:
  void updatePosition();
  //! the camera transformation
  CGE::Matrix camTrafo_;
  //! the distance in front of the camera of the point it orbits
  float mOrbitDistance;
};

#endif
