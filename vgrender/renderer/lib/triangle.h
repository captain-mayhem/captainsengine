#ifndef VR_TRIANGLE_H
#define VR_TRIANGLE_H

#include "common.h"

class VRState;

class Triangle{
public:
  Triangle(float* coords) : mCoords(coords), mIdx0(0), mIdx1(1), mIdx2(2){
  }
  Triangle(float* coords, unsigned idx1, unsigned idx2, unsigned idx3) : mCoords(coords), 
    mIdx0(idx1), mIdx1(idx2), mIdx2(idx3){
  }
  void setVarying(unsigned pos, float* var){
    mVarying[pos] = var;
  }
  void raster(VRState* surface, int numInterpolations);
protected:
  float* mCoords;
  float* mVarying[NUM_VARYING];
  unsigned mIdx0;
  unsigned mIdx1;
  unsigned mIdx2;
};

#endif
