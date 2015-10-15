#include "Screenchange.h"
#include "Engine.h"
#include <system/allocation.h>

TR_CHANNEL(ADV_Screenchange);

using namespace adv;

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

ScreenChangeBase::~ScreenChangeBase(){
  Engine::instance()->getCursor()->showLoading(false);
}

void ScreenChangeBase::screenChangeSwitch(){
  if (mUnload)
    Engine::instance()->forceNotToRenderUnloadingRoom();
}

Screenshot::Screenshot(int depth) : RenderableBlitObject(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, depth){
  realize();
}

void Screenshot::take(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  bind();
  rend->clear(COLORBUFFER | ZBUFFER);
  rend->pushMatrix();
  Engine::instance()->flipTexture();
  Engine::instance()->renderUnloadingRoom();
  rend->popMatrix();
  unbind();
}

CircleScreenChange::CircleScreenChange(int width, int height, int depth, int duration, int segments) : ScreenChangeBase(width, height, depth), mDuration(duration/2), mCurrentTime(0), mClosing(true), mShot(depth-1), mSegments(segments){
  generateCircle(1.0f);
}

CircleScreenChange::~CircleScreenChange(){
  delete mVerts;
}

bool CircleScreenChange::update(unsigned interval){
  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration*Engine::instance()->getResolution().x;
  else
    scale = mCurrentTime/(float)mDuration*Engine::instance()->getResolution().x;
  
  if (mClosing && mCurrentTime == 0){
    mShot.take();
    screenChangeSwitch();
  }

  bind();
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
  rend->enableTexturing(false);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  rend->setColor(0, 0, 0, 0);
  rend->pushMatrix();
  rend->translate(mSize.x/2.0f, mSize.y/2.0f, 0.0f);
  rend->scale(scale, scale*Engine::instance()->getResolution().y/Engine::instance()->getResolution().x, 1.0f);
  mVerts->activate();
  mVerts->draw(CGE::VB_Trifan, NULL);
  rend->popMatrix();
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration && !mClosing){
    return false;
  }
  if (mCurrentTime >= mDuration && mClosing){
    mCurrentTime = 0;
    mClosing = false;
  }
  mCurrentTime += interval;
  if (mClosing)
    mShot.render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

void CircleScreenChange::generateCircle(float radius){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  float angle = (float)M_PI/4.0f;
  mVerts = rend->createVertexBuffer();
  mVerts->create(VB_POSITION | VB_TEXCOORD | VB_NORMAL, (mSegments + 1) + 1);
  mVerts->lockVertexPointer();
  mVerts->setPosition(0, CGE::Vec3f(0.0f, 0.0f, 0.0f));
  for (int i = 1; i < mSegments+2; ++i){
    mVerts->setPosition(i, CGE::Vec3f(radius*cos(angle), radius*sin(angle), 0.0f));
    angle += (float)(M_PI*2./mSegments);
  }
  mVerts->unlockVertexPointer();
}

BlendScreenChange::BlendScreenChange(int width, int height, int depth, int duration) : ScreenChangeBase(width, height, depth), mDuration(duration), mCurrentTime(0), mShot(depth-1){
}

bool BlendScreenChange::update(unsigned interval){
  if (mCurrentTime == 0){
    mShot.take();
    screenChangeSwitch();
  }

  int alpha = (int)((mDuration-mCurrentTime)/(float)mDuration*255);

  bind();
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
  
  //render previous room
  rend->pushMatrix();
  Engine::instance()->flipTexture();
  mShot.blit();
  rend->popMatrix();
  
  //multiply alpha
  rend->pushMatrix();
  rend->scale((float)mSize.x,(float)mSize.y,1.0f);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  rend->setColor(1.0f, 1.0f, 1.0f, alpha / 255.0f);
  rend->enableTexturing(false);
  Engine::instance()->drawQuad();
  rend->popMatrix();
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration){
    return false;
  }
  mCurrentTime += interval;
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

FadeoutScreenChange::FadeoutScreenChange(int width, int height, int depth, int duration) : ScreenChangeBase(width, height, depth), mDuration(duration/2), mCurrentTime(0), mShot(depth-1), mClosing(true){
}

bool FadeoutScreenChange::update(unsigned interval){
  if (mCurrentTime == 0 && mClosing){
    mShot.take();
    screenChangeSwitch();
  }

  int alpha = (int)((mDuration-mCurrentTime)/(float)mDuration*255);
  if (alpha > 255)
    alpha = 255;
  if (alpha < 0)
    alpha = 0;

  bind();
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
  
  //render previous room
  if (mClosing){
    rend->pushMatrix();
    Engine::instance()->flipTexture();
    mShot.blit();
    rend->popMatrix();
  }
  
  //multiply alpha
  rend->pushMatrix();
  rend->scale((float)mSize.x,(float)mSize.y,1.0f);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  if (mClosing)
    rend->setColor(alpha / 255.0f, alpha / 255.0f, alpha / 255.0f, 1.0f);
  else
    rend->setColor(1.0f, 1.0f, 1.0f, alpha / 255.0f);
  rend->enableTexturing(false);
  Engine::instance()->drawQuad();
  rend->popMatrix();
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  unbind();
  if (mCurrentTime >= mDuration && !mClosing){
    return false;
  }
  if (mCurrentTime >= mDuration && mClosing){
    mCurrentTime = 0;
    mClosing = false;
  }
  mCurrentTime += interval;
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

ShuttersScreenChange::ShuttersScreenChange(int width, int height, int depth, int duration) : ScreenChangeBase(width, height, depth), mDuration(duration/2), mCurrentTime(0), mShot(depth-1), mClosing(true){
}

bool ShuttersScreenChange::update(unsigned interval){
  if (mCurrentTime == 0 && mClosing){
    mShot.take();
    screenChangeSwitch();
  }

  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration;//*Engine::instance()->getResolution().y;
  else
    scale = mCurrentTime/(float)mDuration;//*Engine::instance()->getResolution().y;

  bind();
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
  rend->enableTexturing(false);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  rend->setColor(0, 0, 0, 0);
  static const int numshutters = 10;
  for (int i = 0; i < numshutters; ++i){
    float posscale = 2-(i/(numshutters-1.0f));
    rend->pushMatrix();
    rend->translate(0.0f, i*mSize.y/(float)numshutters, 0.0f);
    rend->scale((float)Engine::instance()->getResolution().x, mSize.y/(float)numshutters*scale*posscale, 1.0f);
    Engine::instance()->drawQuad();
    rend->popMatrix();
  }
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration && !mClosing){
    return false;
  }
  if (mCurrentTime >= mDuration && mClosing){
    mCurrentTime = 0;
    mClosing = false;
  }
  mCurrentTime += interval;
  if (mClosing)
    mShot.render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

ClockScreenChange::ClockScreenChange(int width, int height, int depth, int duration, int segments) : ScreenChangeBase(width, height, depth), mDuration(duration/2), mCurrentTime(0), mClosing(true), mShot(depth-1), mSegments(segments){
  generateCircle(1.0f);
}

ClockScreenChange::~ClockScreenChange(){
  delete mVerts;
}

bool ClockScreenChange::update(unsigned interval){
  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration*mSegments;
  else
    scale = mCurrentTime/(float)mDuration*mSegments;
  
  if (mClosing && mCurrentTime == 0){
    mShot.take();
    screenChangeSwitch();
  }

  bind();
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->clear(COLORBUFFER | ZBUFFER);
  rend->enableTexturing(false);
  rend->blendFunc(CGE::BLEND_DST_COLOR, CGE::BLEND_ZERO);
  rend->setColor(0, 0, 0, 0);
  rend->pushMatrix();
  rend->translate(mSize.x/2.0f, mSize.y/2.0f, 0.0f);
  rend->scale(Engine::instance()->getResolution().x*1.0f, Engine::instance()->getResolution().x*1.0f, 1.0f);
  mVerts->activate();
  mVerts->draw(CGE::VB_Trifan, NULL, 0, (int)scale);
  rend->popMatrix();
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration && !mClosing){
    return false;
  }
  if (mCurrentTime >= mDuration && mClosing){
    mCurrentTime = 0;
    mClosing = false;
  }
  mCurrentTime += interval;
  if (mClosing)
    mShot.render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

void ClockScreenChange::generateCircle(float radius){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  float angle = (float)M_PI / 2.0f;
  mVerts = rend->createVertexBuffer();
  mVerts->create(VB_POSITION | VB_TEXCOORD | VB_NORMAL, (mSegments + 1) + 1);
  mVerts->lockVertexPointer();
  mVerts->setPosition(0, CGE::Vec3f(0.0f, 0.0f, 0.0f));
  for (int i = 1; i < mSegments + 2; ++i){
    mVerts->setPosition(i, CGE::Vec3f(radius*cos(angle), radius*sin(angle), 0.0f));
    angle += (float)(M_PI*2. / mSegments);
  }
  mVerts->unlockVertexPointer();
}
