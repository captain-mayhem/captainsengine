#ifndef VGR_PIPELINE_H
#define VGR_PIPELINE_H

#include <system/event.h>
#include <system/thread.h>

#include "common.h"

class Triangle;
class VRState;
class VRShader;

class VRPipeline{
public:
  VRPipeline();
  void rasterTriangle(Triangle* tri, VRState* state, int numInterpolations, const int x, const int y, const int cbypos);
  void setShader(VRShader* shader);
  void waitForRasterIdle();
private:
  static void rasterFunc(void* data);
  struct RasterizerContext{
      VRPipeline* mPipe;
      Triangle* mTri;
      VRState* mState;
      int mNumInterpolations;
      int mX;
      int mY;
      int mCbypos;
      VRShader* mShader;
      CGE::Thread mThread;
      CGE::Event mEvent;
  };
  void rasterTriangle(RasterizerContext* ctx);
  RasterizerContext mRasterCtx[NUM_RASTER_THREADS];
  RasterizerContext* mRasterFreeList[NUM_RASTER_THREADS];
  CGE::Mutex mRasterMutex;
  CGE::Semaphore mRasterSem;
  int mFreeRasterizers;
};

#endif
