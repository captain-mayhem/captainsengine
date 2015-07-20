#include "pipeline.h"

#include "triangle.h"
#include "shader.h"

VRPipeline::VRPipeline() : mRasterSem(NUM_RASTER_THREADS), mFreeRasterizers(NUM_RASTER_THREADS){
  for (int i = 0; i < NUM_RASTER_THREADS; ++i){
    mRasterCtx[i].mPipe = this;
    mRasterCtx[i].mShader = NULL;
    mRasterCtx[i].mThread.create(rasterFunc, &mRasterCtx[i]);
    mRasterFreeList[i] = &mRasterCtx[i];
  }
}

void VRPipeline::setShader(VRShader* shader){
  for (int i = 0; i < NUM_RASTER_THREADS; ++i){
    delete mRasterCtx[i].mShader;
    mRasterCtx[i].mShader = shader->clone();
  }
}

void VRPipeline::waitForRasterIdle(){
  for (int i = 0; i < NUM_RASTER_THREADS; ++i)
    mRasterSem.dec();
  for (int i = 0; i < NUM_RASTER_THREADS; ++i)
    mRasterSem.inc();
}

void VRPipeline::rasterFunc(void* data){
  RasterizerContext* ctx = (RasterizerContext*)data;
  ctx->mPipe->rasterTriangle(ctx);
}

void VRPipeline::rasterTriangle(RasterizerContext* ctx){
  while(true){
    ctx->mEvent.wait();
    ctx->mTri->rasterBlock(ctx->mState, ctx->mShader, ctx->mNumInterpolations, ctx->mX, ctx->mY, ctx->mCbypos);
    mRasterMutex.lock();
    mRasterFreeList[mFreeRasterizers] = ctx;
    ++mFreeRasterizers;
    mRasterSem.inc();
    mRasterMutex.unlock();
  }
}

void VRPipeline::rasterTriangle(Triangle* tri, VRState* state, int numInterpolations, const int x, const int y, const int cbypos){
  mRasterSem.dec();
  mRasterMutex.lock();
  RasterizerContext* ctx = mRasterFreeList[mFreeRasterizers-1];
  ctx->mTri = tri; ctx->mState = state; ctx->mNumInterpolations = numInterpolations; ctx->mX = x; ctx->mY = y; ctx->mCbypos = cbypos;
  --mFreeRasterizers;
  ctx->mEvent.signal();
  mRasterMutex.unlock();
}
