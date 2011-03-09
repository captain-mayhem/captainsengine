#include "Screenchange.h"
#include "Engine.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#define NUM_SEGMENTS 64

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

CircleScreenChange::CircleScreenChange(int width, int height, int depth, int duration) : RenderableBlitObject(width, height, depth), mDuration(duration/2), mCurrentTime(0), mClosing(true), mShot(depth-1){
  generateCircle(1.0f, NUM_SEGMENTS);
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
  GL()drawArrays(GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2);
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

void CircleScreenChange::generateCircle(float radius, int segments){
  float angle = 0;
  mVerts = new float[(segments+1)*2+2];
  mVerts[0] = 0.0f;
  mVerts[1] = 0.0f;
  for (int i = 1; i < segments+2; ++i){
    mVerts[2*i] = radius*cos(angle);
    mVerts[2*i+1] = radius*sin(angle);
    angle += (float)(M_PI*2./segments);
  }
}
