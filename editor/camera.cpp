#include "camera.h"

EditCam::EditCam(){
  camTrafo_ = CGE::Matrix(CGE::Matrix::Translation, CGE::Vec3f(0.0, 0.0, -10.0f));
  updatePosition();
}

void EditCam::updatePosition(){
  CGE::Matrix inv = camTrafo_.inverse();
  mPosition = inv*CGE::Vec3f();
}