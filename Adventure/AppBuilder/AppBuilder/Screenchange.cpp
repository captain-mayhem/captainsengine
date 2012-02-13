#include "Screenchange.h"
#include "Engine.h"

using namespace adv;

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

Screenshot::Screenshot(int depth) : RenderableBlitObject(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, depth){
}

void Screenshot::take(){
  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()pushMatrix();
  GL()translatef(0.0f, (float)Engine::instance()->getResolution().y, 0.0f);
  GL()scalef(1.0f,-1.0f,1.0f);
  Engine::instance()->renderUnloadingRoom();
  GL()popMatrix();
  unbind();
}

CircleScreenChange::CircleScreenChange(int width, int height, int depth, int duration, int segments) : RenderableBlitObject(width, height, depth), mDuration(duration/2), mCurrentTime(0), mClosing(true), mShot(depth-1), mSegments(segments){
  generateCircle(1.0f);
}

CircleScreenChange::~CircleScreenChange(){
  delete [] mVerts;
}

bool CircleScreenChange::update(unsigned interval){
  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration*Engine::instance()->getResolution().x;
  else
    scale = mCurrentTime/(float)mDuration*Engine::instance()->getResolution().x;
  
  if (mClosing && mCurrentTime == 0){
    mShot.take();
  }

  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()disable(GL_TEXTURE_2D);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()color4ub(0, 0, 0, 0);
  GL()pushMatrix();
  GL()translatef(mSize.x/2.0f, mSize.y/2.0f, 0.0f);
  GL()scalef(scale, scale*Engine::instance()->getResolution().y/Engine::instance()->getResolution().x, 1.0f);
  GL()vertexPointer(2, GL_FLOAT, 0, mVerts);
  GL()drawArrays(GL_TRIANGLE_FAN, 0, mSegments+2);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration && !mClosing)
    return false;
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
  float angle = (float)M_PI/4.0f;
  mVerts = new float[(mSegments+1)*2+2];
  mVerts[0] = 0.0f;
  mVerts[1] = 0.0f;
  for (int i = 1; i < mSegments+2; ++i){
    mVerts[2*i] = radius*cos(angle);
    mVerts[2*i+1] = radius*sin(angle);
    angle += (float)(M_PI*2./mSegments);
  }
}

BlendScreenChange::BlendScreenChange(int width, int height, int depth, int duration) : RenderableBlitObject(width, height, depth), mDuration(duration), mCurrentTime(0), mShot(depth-1){
}

bool BlendScreenChange::update(unsigned interval){
  if (mCurrentTime == 0){
    mShot.take();
  }

  int alpha = (int)((mDuration-mCurrentTime)/(float)mDuration*255);

  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //render previous room
  GL()pushMatrix();
  GL()translatef(0.0f, (float)Engine::instance()->getResolution().y, 0.0f);
  GL()scalef(1.0f,-1.0f,1.0f);
  mShot.blit();
  GL()popMatrix();
  
  //multiply alpha
  GL()pushMatrix();
  GL()scalef((float)mSize.x,(float)mSize.y,1.0f);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()color4ub(255, 255, 255, alpha);
  GL()disable(GL_TEXTURE_2D);
  GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration)
    return false;
  mCurrentTime += interval;
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

FadeoutScreenChange::FadeoutScreenChange(int width, int height, int depth, int duration) : RenderableBlitObject(width, height, depth), mDuration(duration/2), mCurrentTime(0), mShot(depth-1), mClosing(true){
}

bool FadeoutScreenChange::update(unsigned interval){
  if (mCurrentTime == 0 && mClosing){
    mShot.take();
  }

  int alpha = (int)((mDuration-mCurrentTime)/(float)mDuration*255);
  if (alpha > 255)
    alpha = 255;
  if (alpha < 0)
    alpha = 0;

  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //render previous room
  if (mClosing){
    GL()pushMatrix();
    GL()translatef(0.0f, (float)Engine::instance()->getResolution().y, 0.0f);
    GL()scalef(1.0f,-1.0f,1.0f);
    mShot.blit();
    GL()popMatrix();
  }
  
  //multiply alpha
  GL()pushMatrix();
  GL()scalef((float)mSize.x,(float)mSize.y,1.0f);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  if (mClosing)
    GL()color4ub(alpha, alpha, alpha, 255);
  else
    GL()color4ub(255, 255, 255, alpha);
  GL()disable(GL_TEXTURE_2D);
  GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  unbind();
  if (mCurrentTime >= mDuration && !mClosing)
    return false;
  if (mCurrentTime >= mDuration && mClosing){
    mCurrentTime = 0;
    mClosing = false;
  }
  mCurrentTime += interval;
  render(Vec2i(0,0), Vec2f(1.0f,1.0f), Vec2i(0,0));
  return true;
}

ShuttersScreenChange::ShuttersScreenChange(int width, int height, int depth, int duration) : RenderableBlitObject(width, height, depth), mDuration(duration/2), mCurrentTime(0), mShot(depth-1), mClosing(true){
}

bool ShuttersScreenChange::update(unsigned interval){
  if (mCurrentTime == 0 && mClosing){
    mShot.take();
  }

  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration;//*Engine::instance()->getResolution().y;
  else
    scale = mCurrentTime/(float)mDuration;//*Engine::instance()->getResolution().y;

  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()disable(GL_TEXTURE_2D);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()color4ub(0, 0, 0, 0);
  static const int numshutters = 10;
  for (int i = 0; i < numshutters; ++i){
    float posscale = 2-(i/(numshutters-1.0f));
    GL()pushMatrix();
    GL()translatef(0.0f, i*mSize.y/(float)numshutters, 0.0f);
    GL()scalef((float)Engine::instance()->getResolution().x, mSize.y/(float)numshutters*scale*posscale, 1.0f);
    GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    GL()popMatrix();
  }
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration && !mClosing)
    return false;
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

ClockScreenChange::ClockScreenChange(int width, int height, int depth, int duration, int segments) : RenderableBlitObject(width, height, depth), mDuration(duration/2), mCurrentTime(0), mClosing(true), mShot(depth-1), mSegments(segments){
  generateCircle(1.0f);
}

ClockScreenChange::~ClockScreenChange(){
  delete [] mVerts;
}

bool ClockScreenChange::update(unsigned interval){
  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration*mSegments;
  else
    scale = mCurrentTime/(float)mDuration*mSegments;
  
  if (mClosing && mCurrentTime == 0){
    mShot.take();
  }

  bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()disable(GL_TEXTURE_2D);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()color4ub(0, 0, 0, 0);
  GL()pushMatrix();
  GL()translatef(mSize.x/2.0f, mSize.y/2.0f, 0.0f);
  GL()scalef(Engine::instance()->getResolution().x*1.0f, Engine::instance()->getResolution().x*1.0f, 1.0f);
  GL()vertexPointer(2, GL_FLOAT, 0, mVerts);
  GL()drawArrays(GL_TRIANGLE_FAN, 0, (GLsizei)scale);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  unbind();
  
  if (mCurrentTime >= mDuration && !mClosing)
    return false;
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
  float angle = (float)M_PI/2.0f;
  mVerts = new float[(mSegments+1)*2+2];
  mVerts[0] = 0.0f;
  mVerts[1] = 0.0f;
  for (int i = 1; i < mSegments+2; ++i){
    mVerts[2*i] = radius*cos(angle);
    mVerts[2*i+1] = radius*sin(angle);
    angle += (float)(M_PI*2./mSegments);
  }
}
