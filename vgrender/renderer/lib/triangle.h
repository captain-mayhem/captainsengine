#ifndef VR_TRIANGLE_H
#define VR_TRIANGLE_H

#include "common.h"

class VRState;
class VRShader;

class Triangle{
public:
  Triangle(float* coords) : mCoords(coords), mIdx0(0), mIdx1(1), mIdx2(2), q(32){
  }
  Triangle(float* coords, unsigned idx1, unsigned idx2, unsigned idx3) : mCoords(coords), 
    mIdx0(idx1), mIdx1(idx2), mIdx2(idx3), q(32){
  }
  void setVarying(unsigned pos, float* var){
    mVarying[pos] = var;
  }
  void raster(VRState* surface, int numInterpolations);
  void rasterBlock(VRState* state, VRShader* shader, int numInterpolations, const int x, const int y, const int cbypos) const;
protected:
  float* mCoords;
  float* mVarying[NUM_VARYING];
  unsigned mIdx0;
  unsigned mIdx1;
  unsigned mIdx2;

  //raster state
  //block size, must be power of two
  const int q;
  int minx;
  int miny;
  int fAlpha;
  //barycentric values
  int C1;
  int C2;
  int C3;
  int DX12;
  int DX23;
  int DX31;
  int DY12;
  int DY23;
  int DY31;
  int FDX12;
  int FDX23;
  int FDX31;
  int FDY12;
  int FDY23;
  int FDY31;
  //data interpolation
  int interp_dx[NUM_VARYING*4];
  int interp_dy[NUM_VARYING*4];
  int interp_starts[NUM_VARYING*4];
};

#endif
