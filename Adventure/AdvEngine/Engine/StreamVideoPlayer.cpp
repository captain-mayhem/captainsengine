#include "StreamVideoPlayer.h"

#include <io/Tracing.h>
#include <video/video.h>

#include "BlitObjects.h"
#include "Engine.h"
#include "ExecutionContext.h"
#include "Ids.h"

using namespace adv;

TR_CHANNEL(ADV_Video);

StreamVideoPlayer::StreamVideoPlayer(const std::string& name, std::unique_ptr<CGE::Video> video) : mName(name), mVideo(std::move(video)),
mLayer(nullptr), mFrameShown(0), mScale(1.0f, 1.0f), mSuspensionScript(nullptr), mLooping(false), mStop(true){
  TR_USE(ADV_Video);
  TR_DEBUG("Video %s: %ix%i", name.c_str(), mVideo->getWidth(), mVideo->getHeight());
}

StreamVideoPlayer::~StreamVideoPlayer(){
  mVideo->stop();
  if (mSuspensionScript){
    mSuspensionScript->resume();
    ExecutionContext::unref(mSuspensionScript);
    mSuspensionScript = nullptr;
  }
  delete mLayer;
}

void StreamVideoPlayer::play(bool looping){
  mLooping = looping;
  mStop = false;
  mVideo->play();
}

void StreamVideoPlayer::stop(){
  mStop = true;
  mVideo->stop();
}

bool StreamVideoPlayer::update(unsigned time){
  if (mStop)
    return false;
  //escape in the cutscene that waits for the video
  if (mSuspensionScript && mSuspensionScript->isSkipping()){
    stop();
    return false;
  }
  if (!mVideo->update(time/1000.0)){
    if (!mLooping){
      mStop = true;
      return false;
    }
    mVideo->play();
  }
  return true;
}

void StreamVideoPlayer::render(unsigned time){
  if (!mLayer || mStop || !mVideo->getPixels())
    return;
  //one texture is enough: the picture changes here, before the layer is drawn this frame
  if (mFrameShown != mVideo->getFrameCount()){
    mLayer->updateTexture(mVideo->getWidth(), mVideo->getHeight(), (void*)mVideo->getPixels());
    mFrameShown = mVideo->getFrameCount();
  }
  Engine::instance()->beginRendering();
  mLayer->render(mRenderPos, mScale, Vec2i());
  Engine::instance()->endRendering();
}

void StreamVideoPlayer::initLayer(int x, int y, int width, int height){
  mRenderPos.x = x;
  mRenderPos.y = y;
  mScale.x = width/(float)mVideo->getWidth();
  mScale.y = height/(float)mVideo->getHeight();
  delete mLayer;
  mLayer = new BlitObject(mVideo->getWidth(), mVideo->getHeight(), DEPTH_VIDEO_LAYER);
  mLayer->realizeEmpty(CGE::Gfx::Format::RGBA8);
  mFrameShown = 0;
}

void StreamVideoPlayer::setSuspensionScript(ExecutionContext* ctx){
  if (mSuspensionScript != nullptr){
    mSuspensionScript->reset(true, true);
    ExecutionContext::unref(mSuspensionScript);
  }
  ctx->ref();
  mSuspensionScript = ctx;
}
