#ifndef VR_SHADER_H
#define VR_SHADER_H

#include "common.h"

class VRSurface;

class VRShader{
public:
  void shade(VRSurface* surface, int x, int y);
  void setVarying(int idx, float* arr) {mVarying[idx] = arr;}
  virtual int getVarying(const char* name) {return -1;}
  virtual unsigned getNumVaryings() {return 0;}
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
public:
  virtual int getVarying(const char* name);
  virtual unsigned getNumVaryings();
protected:
  virtual void execute();
};

#endif
