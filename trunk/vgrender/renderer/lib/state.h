#ifndef VR_STATE_H
#define VR_STATE_H

#include <vector>

#include "common.h"
#include "pipeline.h"

class VRSurface;
class VRShader;

class VRState{
public:
  VRState();
  ~VRState();
  void setClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a){
    mClearColor[0] = r; mClearColor[1] = g; mClearColor[2] = b; mClearColor[3] = a;
  }
  unsigned char* getClearColor() {return mClearColor;}
  void setSurface(VRSurface* surface) {mSurface = surface;}
  VRSurface* getCurrentSurface() {return mSurface;}
  void setProgram(VRShader* shader) {mShader = shader; mPipeline->setShader(shader);}
  VRShader* getActiveShader() {return mShader;}
  void enableVertexAttribArray(unsigned idx, bool enable){
    mVertexAttribArrays[idx] = enable;
  }
  bool isVertexAttribArrayEnabled(unsigned idx){ return mVertexAttribArrays[idx];}
  void setVertexArribPointer(unsigned idx, float* attribs, int numComponents){
    mVertexAttribPointers[idx] = attribs;
  }
  void setVertices(float* verts) {mVertices = verts;}
  float* getVertices() {return mVertices;}
  void drawTriangles(int first, int count);
  unsigned makeShaderSlot() {mShaders.push_back(NULL); return mShaders.size()-1;}
  unsigned makeProgramSlot() {mPrograms.push_back(NULL); return mPrograms.size()-1;}
  void setShader(unsigned idx, VRShader* shader) {mShaders[idx] = shader;}
  VRShader* getShader(unsigned idx) {return mShaders[idx];}
  void addToProgram(unsigned idx, VRShader* shader) {mPrograms[idx] = shader;}
  VRShader* getProgram(unsigned idx) {return mPrograms[idx];}
  void setError(int error) {mError = error;}
  VRPipeline* getPipeline() {return mPipeline;}
private:
  VRPipeline* mPipeline;
  VRSurface* mSurface;
  VRShader* mShader;
  bool mVertexAttribArrays[NUM_VARYING];
  float* mVertexAttribPointers[NUM_VARYING];
  unsigned char mClearColor[4];
  float* mVertices;
  std::vector<VRShader*> mShaders;
  std::vector<VRShader*> mPrograms;
  int mError;
};

extern VRState* vrGlobalState;
#define VR_CONTEXT vrGlobalState

#endif
