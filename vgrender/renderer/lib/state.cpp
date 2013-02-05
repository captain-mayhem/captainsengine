#include "state.h"

#include <cstdlib>
#include <cstring>

#include "triangle.h"
#include "shader.h"
#include "pipeline.h"

VRState* vrGlobalState = NULL;

VRState::VRState() : mPipeline(NULL), mSurface(NULL), mShader(NULL), mVertices(NULL), mError(0){
  mPipeline = new VRPipeline;
  vrGlobalState = this;
  for (int i = 0; i < NUM_VARYING; ++i){
    mVertexAttribArrays[i] = false;
    mVertexAttribPointers[i] = NULL;
  }
  memset(mClearColor, 0, 4*sizeof(unsigned char));
}

VRState::~VRState(){
  delete mPipeline;
}

void VRState::drawTriangles(int first, int count){
  unsigned numVarying = mShader->getNumVaryings();
  int numTris = count/3;
  for (int i = 0; i < numTris; ++i){
    Triangle t(mVertices+i*2*3);
    for (unsigned j = 0; j < numVarying; ++j){
      t.setVarying(j, mVertexAttribPointers[j]+i*4*3);
    }
    t.raster(this, numVarying);
  }
}
