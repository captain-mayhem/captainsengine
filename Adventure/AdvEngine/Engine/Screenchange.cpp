#include "Screenchange.h"
#include "Engine.h"
#include "Graphics.h"
#include <math/matrixstack.h>
#include <renderer/renderdevice.h>
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
  Graphics* gfx = Engine::instance()->getGraphics();
  bind();
  gfx->clear();
  gfx->model().push();
  Engine::instance()->flipTexture();
  Engine::instance()->renderUnloadingRoom();
  gfx->model().pop();
  unbind();
}

CircleScreenChange::CircleScreenChange(int width, int height, int depth, int duration, int segments) : ScreenChangeBase(width, height, depth), mDuration(duration/2), mCurrentTime(0), mClosing(true), mShot(depth-1), mSegments(segments){
  generateCircle(1.0f);
}

CircleScreenChange::~CircleScreenChange(){
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
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->clear();
  gfx->setTexture(nullptr);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  gfx->setColor(0, 0, 0, 0);
  gfx->model().push();
  gfx->model().translate(mSize.x/2.0f, mSize.y/2.0f, 0.0f);
  gfx->model().scale(scale, scale*Engine::instance()->getResolution().y/Engine::instance()->getResolution().x, 1.0f);
  gfx->draw(mVerts.get(), CGE::Gfx::PrimitiveType::TriangleStrip, mSegments*2+1);
  gfx->model().pop();
  gfx->setBlend(Graphics::BLEND_ALPHA);
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
  mVerts = createCircle(radius, (float)M_PI/4.0f, mSegments);
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
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->clear();

  //render previous room
  gfx->model().push();
  Engine::instance()->flipTexture();
  mShot.blit();
  gfx->model().pop();

  //multiply alpha
  gfx->model().push();
  gfx->model().scale((float)mSize.x,(float)mSize.y,1.0f);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  gfx->setColor(1.0f, 1.0f, 1.0f, alpha / 255.0f);
  gfx->setTexture(nullptr);
  gfx->drawQuad();
  gfx->model().pop();
  gfx->setBlend(Graphics::BLEND_ALPHA);
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
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->clear();

  //render previous room
  if (mClosing){
    gfx->model().push();
    Engine::instance()->flipTexture();
    mShot.blit();
    gfx->model().pop();
  }

  //multiply alpha
  gfx->model().push();
  gfx->model().scale((float)mSize.x,(float)mSize.y,1.0f);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  if (mClosing)
    gfx->setColor(alpha / 255.0f, alpha / 255.0f, alpha / 255.0f, 1.0f);
  else
    gfx->setColor(1.0f, 1.0f, 1.0f, alpha / 255.0f);
  gfx->setTexture(nullptr);
  gfx->drawQuad();
  gfx->model().pop();
  gfx->setBlend(Graphics::BLEND_ALPHA);
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
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->clear();
  gfx->setTexture(nullptr);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  gfx->setColor(0, 0, 0, 0);
  static const int numshutters = 10;
  for (int i = 0; i < numshutters; ++i){
    float posscale = 1+(i/(numshutters-1.0f));
    gfx->model().push();
    Engine::instance()->flipTexture();
    gfx->model().translate(0.0f, (i+1)*mSize.y/(float)numshutters, 0.0f);
    gfx->model().scale((float)Engine::instance()->getResolution().x, mSize.y/(float)numshutters*-scale*posscale, 1.0f);
    gfx->drawQuad();
    gfx->model().pop();
  }
  gfx->setBlend(Graphics::BLEND_ALPHA);
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
}

bool ClockScreenChange::update(unsigned interval){
  float scale;
  if (mClosing)
    scale = (mDuration-mCurrentTime)/(float)mDuration*(2*mSegments+1);
  else
    scale = mCurrentTime/(float)mDuration*(2*mSegments+1);
  if (scale > 2 * mSegments + 1)
    scale = (float)(2 * mSegments + 1);
  if (scale < 0)
    scale = 0;
  
  if (mClosing && mCurrentTime == 0){
    mShot.take();
    screenChangeSwitch();
  }

  bind();
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->clear();
  gfx->setTexture(nullptr);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  gfx->setColor(0, 0, 0, 0);
  gfx->model().push();
  Engine::instance()->flipTexture();
  gfx->model().translate(mSize.x/2.0f, mSize.y/2.0f, 0.0f);
  gfx->model().scale(Engine::instance()->getResolution().x*1.0f, Engine::instance()->getResolution().x*1.0f, 1.0f);
  gfx->draw(mVerts.get(), CGE::Gfx::PrimitiveType::TriangleStrip, (int)scale);
  gfx->model().pop();
  gfx->setBlend(Graphics::BLEND_ALPHA);
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
  mVerts = createCircle(radius, -(float)M_PI/2.0f, mSegments);
}

std::unique_ptr<CGE::Gfx::Buffer> adv::createCircle(float radius, float angle, int segments){
  //a triangle strip alternating between the rim and the centre
  std::vector<Graphics::Vertex> verts((segments * 2) + 1);
  verts[0] = {radius*cosf(angle), radius*sinf(angle), 0.0f, 0.0f, 0.0f};
  for (int i = 1; i <= segments; ++i){
    angle += (float)(M_PI*2. / segments);
    verts[2*i - 1] = {radius*cosf(angle), radius*sinf(angle), 0.0f, 0.0f, 0.0f};
    verts[2 * i] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  }
  return Engine::instance()->getGraphics()->createVertices((int)verts.size(), verts.data(), CGE::Gfx::Usage::Immutable);
}
