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
private:
  void updatePosition();
  //! the camera transformation
  CGE::Matrix camTrafo_;
};

#endif