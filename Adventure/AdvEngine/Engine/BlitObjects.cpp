#include "BlitObjects.h"

#include <math/matrixstack.h>
#include <renderer/renderdevice.h>

#include "Engine.h"
#include "Graphics.h"

#include <system/allocation.h>

TR_CHANNEL(ADV_Render_BlitObject);

using namespace adv;

BaseBlitObject::BaseBlitObject(int depth, const Vec2i& size) :
mPos(), mSize(size), mDepth(depth){

}

BaseBlitObject::~BaseBlitObject(){

}

BlitObject::BlitObject(int width, int height, int depth) :
BaseBlitObject(depth, Vec2i(width, height)), mOffset(), mScale(1.0f, 1.0f), mTex(0), mImage(nullptr), mMirrorOffset(), mRotAngle(0.0f), mBlendMode(BLEND_ALPHA){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
}

BlitObject::BlitObject(std::string texture, const Vec2i& size, int depth, Vec2i offset) :
BaseBlitObject(depth, Vec2i()), mOffset(offset), mScale(1.0f, 1.0f), mTex(0), mMirrorOffset(), mRotAngle(0.0f), mBlendMode(BLEND_ALPHA){
  mImage = Engine::instance()->getImage(texture);
	if (mImage && size > Vec2i()){
		mImage->crop(-offset.x, -offset.y, size.x, size.y);
		if (mOffset.x < 0)
			mOffset.x = 0;
		if (mOffset.y < 0)
			mOffset.y = 0;
	}
  else if (mImage){
    mSize = Vec2i(mImage->getWidth(), mImage->getHeight());
  }
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
}

BlitObject::BlitObject(CGE::Gfx::Texture* texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
BaseBlitObject(depth, size), mOffset(offset), mScale(scale), mTex(texture), mImage(nullptr), mMirrorOffset(), mRotAngle(0), mBlendMode(BLEND_ALPHA)
{
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = false;
}

BlitObject::~BlitObject(){
  delete mImage;
  if (mDeleteTex)
    delete mTex;
}

bool BlitObject::operator<(const BlitObject& obj){
  return mDepth < obj.mDepth;
}

void BlitObject::render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize){
  mMirrorOffset = parentsize;
  mPos.x = mOffset.x+pos.x;
  mPos.y = mOffset.y+pos.y;
  mZoomScale = scale;
  Engine::instance()->insertToBlit(this);
}

void BlitObject::blit(){
  if (!mTex)
    return;
  Graphics* gfx = Engine::instance()->getGraphics();
  if (mBlendMode == BLEND_ADDITIVE)
    gfx->setBlend(Graphics::BLEND_ADDITIVE);
  else if (mBlendMode == BLEND_PREMULT_ALPHA)
    gfx->setBlend(Graphics::BLEND_PREMULT_ALPHA);
  CGE::MatrixStack& model = gfx->model();
  model.push();

  if (mZoomScale.x < 0){
    model.translate(mMirrorOffset.x*-mZoomScale.x, 0.0f, 0.0f);
  }
  if (mZoomScale.y < 0){
    model.translate(0.0f, mMirrorOffset.y*-mZoomScale.y, 0.0f);
  }

  model.translate((float)mPos.x, (float)mPos.y, 0.0f);

  model.translate((float)-mOffset.x, (float)-mOffset.y, 0.0f);
  model.scale(mZoomScale.x, mZoomScale.y, 1.0f);
  model.translate((float)mOffset.x, (float)mOffset.y, 0.0f);

  //TODO wrong for compound objects, maybe rotation of mOffset?
  model.translate((float)mSize.x/2,(float)mSize.y/2, 0.0f);
  model.rotate(mRotAngle, 0, 0, 1.0f);
  model.translate((float)-mSize.x/2,(float)-mSize.y/2, 0.0f);

  model.scale((float)mSize.x,(float)mSize.y,1.0f);

  gfx->drawSprite(mTex, mScale, Vec2f(0, 0), mColor.r/255.0f, mColor.g/255.0f, mColor.b/255.0f, mColor.a/255.0f);
  model.pop();
  if (mBlendMode != BLEND_ALPHA)
    gfx->setBlend(Graphics::BLEND_ALPHA);
}

BlitObject* BlitObject::clone(){
  BlitObject* obj = new BlitObject(mTex, mSize, mScale, mDepth, mOffset);
  return obj;
}

void BlitObject::updateTexture(unsigned width, unsigned height, void* data){
  if (!mTex)
    return;
  CGE::Gfx::TextureRegion region;
  region.width = (int)width;
  region.height = (int)height;
  Engine::instance()->getGraphics()->getDevice().updateTexture(mTex, region, data);
}

void BlitObject::realize(){
  if (mImage == nullptr)
    return;
  mTex = Engine::instance()->genTexture(mImage, mSize, mScale);
  delete mImage;
  mImage = nullptr;
}

void BlitObject::realizeEmpty(CGE::Gfx::Format format){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  CGE::Gfx::TextureDesc desc;
  desc.format = format;
  desc.width = pow2.x;
  desc.height = pow2.y;
  mTex = Engine::instance()->getGraphics()->getDevice().createTexture(desc).release();
}

LightingBlitObject::LightingBlitObject(int depth, const Vec2i& size) : BaseBlitObject(depth, size){
}

LightingBlitObject::~LightingBlitObject(){

}

void LightingBlitObject::render(const Vec2i& pos){
  mPos = pos;
  Engine::instance()->insertToBlit(this);
}

void LightingBlitObject::blit(){
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->setTexture(nullptr);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  gfx->model().push();
  gfx->model().translate((float)mPos.x,(float)mPos.y,0.0f);
  gfx->model().scale((float)mSize.x,(float)mSize.y,1.0f);
  gfx->setColor(mColor.r / 255.0f, mColor.g / 255.0f, mColor.b / 255.0f, mColor.a / 255.0f);
  gfx->drawQuad();
  gfx->setColor(1.0f, 1.0f, 1.0f, 1.0f);
  gfx->setBlend(Graphics::BLEND_ALPHA);
  gfx->model().pop();
}

ScrollBlitObject::ScrollBlitObject(int depth) : BaseBlitObject(depth, Vec2i(0,0)){
}

ScrollBlitObject::~ScrollBlitObject(){
}

void ScrollBlitObject::blit(){
  CGE::MatrixStack& model = Engine::instance()->getGraphics()->model();
  if (mDepth < 0){
    model.push();
    model.translate((float)mPos.x, (float)mPos.y, 0);
  }
  else
    model.pop();
}

void ScrollBlitObject::render(const Vec2i& pos){
  mPos = pos;
}

RenderableBlitObject::RenderableBlitObject(int width, int height, int depth) : BlitObject(width,height,depth){
}

void RenderableBlitObject::realize(){
  TR_USE(ADV_Render_BlitObject);
  CGE::Gfx::RenderTargetDesc desc;
  desc.width = Engine::roundToPowerOf2(mSize.x);
  desc.height = Engine::roundToPowerOf2(mSize.y);
  desc.depthFormat = CGE::Gfx::Format::Depth24;
  mFrameBuffer = Engine::instance()->getGraphics()->getDevice().createRenderTarget(desc);
  if (!mFrameBuffer){
    TR_ERROR("Unable to create framebuffer");
    return;
  }
  //the texture belongs to the render target
  mTex = mFrameBuffer->getColorTexture(0);
  mDeleteTex = false;
}

RenderableBlitObject::~RenderableBlitObject(){
}

void RenderableBlitObject::bind(){
  if (mFrameBuffer)
    Engine::instance()->getGraphics()->pushTarget(mFrameBuffer.get(), mSize);
}

void RenderableBlitObject::unbind(){
  if (mFrameBuffer)
    Engine::instance()->getGraphics()->popTarget();
}

DynamicAnimation::DynamicAnimation(){

}

DynamicAnimation::~DynamicAnimation(){

}

MirrorObject::MirrorObject(int width, int height, int depth, unsigned char strength) :
RenderableBlitObject(width, height, depth), mOpacity(strength), mIsWallMirror(false){
  memset(mPoints, 0, 4 * sizeof(Vec2i));
}

MirrorObject::~MirrorObject(){
}

bool MirrorObject::update(unsigned interval){
  Graphics* gfx = Engine::instance()->getGraphics();
  CGE::MatrixStack& model = gfx->model();
  bind();
  //setup
  gfx->setTexture(nullptr);
  gfx->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  gfx->setClearDepth(0.0f);
  gfx->clear();
  gfx->setClearDepth(1.0f);

  //draw mirror surface, into the depth buffer only
  gfx->setDepth(true, true, CGE::Gfx::CompareFunc::GreaterEqual);
  gfx->setColorWrite(false);
  gfx->setColor(1.0f, 1.0f, 0.0f, 1.0f);
  model.push();
  model.loadIdentity();
  Engine::instance()->flipTexture();
  model.push();
  model.translate((float)mRoom->getScrollOffset().x, (float)mRoom->getScrollOffset().y, 0.0f);
  gfx->draw(mPolygon.get(), CGE::Gfx::PrimitiveType::TriangleStrip, 5);
  model.pop();
  gfx->setColorWrite(true);

  //render characters where the surface is
  gfx->setDepth(true, false, CGE::Gfx::CompareFunc::Equal);
  gfx->setBlend(Graphics::BLEND_ALPHA);
  Engine::instance()->beginRendering();
  CharacterObject* self = Engine::instance()->getCharacter("self");
  if (self)
    renderCharacter(self);
  for (int i = (int)mRoom->getObjects().size()-1; i >= 0; --i){
    if (mRoom->getObjects()[i]->getType() == Object2D::CHARACTER){
      renderCharacter((CharacterObject*)mRoom->getObjects()[i]);
    }
  }
  Engine::instance()->endRendering(true);
  gfx->setDepth(false, false);

  //multiply alpha
  model.scale((float)mSize.x,(float)mSize.y,1.0f);
  gfx->setBlend(Graphics::BLEND_MULTIPLY);
  gfx->setColor(1.0f, 1.0f, 1.0f, mOpacity / 255.0f);
  gfx->setTexture(nullptr);
  gfx->drawQuad();
  model.pop();

  //restore
  gfx->setBlend(Graphics::BLEND_ALPHA);
  unbind();
  return true;
}

void MirrorObject::setMirrorArea(Vec2i points[4], RoomObject* room){
  mRoom = room;
  int xmin = 1000; int xmax = -1000;
  for (int i = 0; i < 4; ++i){
    if (points[i].x > xmax)
      xmax = points[i].x;
    if (points[i].x < xmin)
      xmin = points[i].x;
  }
  mMirrorCenter = (xmin + xmax) / 2;
  memcpy(mPoints, points, 4 * sizeof(Vec2i));
}

void MirrorObject::renderCharacter(CharacterObject* chr){
  if (mIsWallMirror){
    if (chr->getDepth() < mDepth)
      return;
    int oldstate = chr->getState();
    Vec2i oldpos = chr->getPosition();
    chr->updateState(true, false);
    Vec2i newpos = chr->getPosition();
    int mirrorbase = (mDepth+1)*(int)Engine::instance()->getWalkGridSize(false);
    newpos.y = mirrorbase + mMirrorOffset.y;
    int diff = newpos.y-oldpos.y;
    newpos.y += diff;
    if (mPositionDependent){
      int xdiff = mMirrorCenter-newpos.x;
      float diffadapt = float(chr->getPosition().y-mDepth*Engine::instance()->getWalkGridSize(false));
      diffadapt /= chr->getWalkGridSize();
      diffadapt = 10 - diffadapt;
      if (diffadapt <= 2)
        diffadapt = 2;
      newpos.x += (int)(xdiff/diffadapt);
    }
    else{
      newpos.x += mMirrorOffset.x;
    }
    chr->setPosition(newpos);
    chr->render();
    chr->setPosition(oldpos);
    chr->setState(oldstate);
  }
  else{
    chr->render(true);
  }
}

void MirrorObject::setWallMirror(Vec2i offset, bool positionDependent){
  mMirrorOffset = offset;
  mPositionDependent = positionDependent;
  mIsWallMirror = true;
}

void MirrorObject::realize(){
  RenderableBlitObject::realize();
  //a triangle strip around the four points
  const int order[5] = {1, 2, 0, 3, 0};
  Graphics::Vertex polygon[5];
  for (int i = 0; i < 5; ++i)
    polygon[i] = {(float)mPoints[order[i]].x, (float)mPoints[order[i]].y, 0.0f, 0.0f, 0.0f};
  mPolygon = Engine::instance()->getGraphics()->createVertices(5, polygon, CGE::Gfx::Usage::Immutable);
}
