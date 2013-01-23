#include "state.h"
#include "triangle.h"

#include <cstdlib>
#include <cstring>

VRState* vrGlobalState = NULL;

VRState::VRState() : mSurface(NULL), mShader(NULL){
  vrGlobalState = this;
  for (int i = 0; i < NUM_VARYING; ++i)
    mVertexAttribArrays[i] = false;
  memset(mClearColor, 0, 4*sizeof(unsigned char));
}

void VRState::drawTriangles(int first, int count){
  int numTris = count/3;
  for (int i = 0; i < numTris; ++i){
    Triangle t(mVertices);
    t.raster(this, 1);
  }
}
