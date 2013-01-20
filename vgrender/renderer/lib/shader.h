#ifndef VR_SHADER_H
#define VR_SHADER_H

#define NUM_VARYING 1

class VRShader{
public:
  virtual void execute() = 0;
  void setVarying(int idx, float* arr);
protected:
  float* mVarying[NUM_VARYING];
};

class GouraudShader : public VRShader{
public:
  virtual void execute();
};

#endif
