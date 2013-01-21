#ifndef VR_STATE_H
#define VR_STATE_H

#include "common.h"

class VRSurface;
class VRShader;

class VRState{
public:
  VRState();
  void setSurface(VRSurface* surface) {mSurface = surface;}
  VRSurface* getCurrentSurface() {return mSurface;}
  void setShader(VRShader* shader) {mShader = shader;}
  VRShader* getActiveShader() {return mShader;}
  void enableVertexAttribArray(unsigned idx, bool enable){
    mVertexAttribArrays[idx] = enable;
  }
  bool isVertexAttribArrayEnabled(unsigned idx){ return mVertexAttribArrays[idx];}
private:
  VRSurface* mSurface;
  VRShader* mShader;
  bool mVertexAttribArrays[NUM_VARYING];
};

#endif
