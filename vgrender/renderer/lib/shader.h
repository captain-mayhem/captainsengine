#ifndef VR_SHADER_H
#define VR_SHADER_H

#include "common.h"

class VRSurface;

class VRShader{
public:
  void shade(VRSurface* surface, int x, int y);
  void setVarying(int idx, float* arr) {mVarying[idx] = arr;}
protected:
  virtual void execute() = 0;
  float mColor[4]; //write only
  float* mVarying[NUM_VARYING]; //read only
};

class FlatShader : public VRShader{
protected:
  virtual void execute();
};

class GouraudShader : public VRShader{
protected:
  virtual void execute();
};

#endif
