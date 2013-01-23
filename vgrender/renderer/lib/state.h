#ifndef VR_STATE_H
#define VR_STATE_H

#include "common.h"

class VRSurface;
class VRShader;

class VRState{
public:
  VRState();
  void setClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a){
    mClearColor[0] = r; mClearColor[1] = g; mClearColor[2] = b; mClearColor[3] = a;
  }
  unsigned char* getClearColor() {return mClearColor;}
  void setSurface(VRSurface* surface) {mSurface = surface;}
  VRSurface* getCurrentSurface() {return mSurface;}
  void setShader(VRShader* shader) {mShader = shader;}
  VRShader* getActiveShader() {return mShader;}
  void enableVertexAttribArray(unsigned idx, bool enable){
    mVertexAttribArrays[idx] = enable;
  }
  bool isVertexAttribArrayEnabled(unsigned idx){ return mVertexAttribArrays[idx];}
  void setVertices(float* verts) {mVertices = verts;}
  float* getVertices() {return mVertices;}
  void drawTriangles(int first, int count);
private:
  VRSurface* mSurface;
  VRShader* mShader;
  bool mVertexAttribArrays[NUM_VARYING];
  unsigned char mClearColor[4];
  float* mVertices;
};

extern VRState* vrGlobalState;
#define VR_CONTEXT vrGlobalState

#endif
