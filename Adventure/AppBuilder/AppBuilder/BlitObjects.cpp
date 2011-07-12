#include "BlitObjects.h"

#include <system/engine.h>

#include "Engine.h"

BaseBlitObject::BaseBlitObject(int depth, const Vec2i& size) : 
mPos(), mSize(size), mDepth(depth){

}

BaseBlitObject::~BaseBlitObject(){

}

BlitObject::BlitObject(int width, int height, int depth) : 
BaseBlitObject(depth, Vec2i(width, height)), mOffset(), mMirrorOffset(), mRotAngle(0.0f){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
  glGenTextures(1, &mTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
BaseBlitObject(depth, Vec2i()), mOffset(offset), mMirrorOffset(), mRotAngle(0.0f){
  CGE::Image* image = Engine::instance()->getImage(texture);
  mTex = Engine::instance()->genTexture(image, mSize, mScale);
  delete image;
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
}

BlitObject::BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
BaseBlitObject(depth, size), mOffset(offset), mScale(scale), mTex(texture), mMirrorOffset(), mRotAngle(0)
{
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = false;
}

BlitObject::~BlitObject(){
  if (mDeleteTex)
    glDeleteTextures(1, &mTex);
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
  GL()pushMatrix();
  
  if (mZoomScale.x < 0){
    GL()translatef(mMirrorOffset.x*-mZoomScale.x,0.0f,0.0f);
  }

  GL()translatef((GLfloat)mPos.x,(GLfloat)mPos.y,0.0f);
  
  GL()translatef((float)-mOffset.x, (float)-mOffset.y, 0.0f);
  GL()scalef(mZoomScale.x, mZoomScale.y, 1.0f);
  GL()translatef((float)mOffset.x, (float)mOffset.y, 0.0f);
  
  //TODO wrong for compound objects, maybe rotation of mOffset?
  GL()translatef((float)mSize.x/2,(float)mSize.y/2, 0.0f);
  GL()rotatef(mRotAngle, 0, 0, 1.0f);
  GL()translatef((float)-mSize.x/2,(float)-mSize.y/2, 0.0f);

  GL()scalef((float)mSize.x,(float)mSize.y,1.0f);
  
  GL()matrixMode(MM_TEXTURE);
  GL()loadIdentity();
  GL()scalef(mScale.x, mScale.y, 1.0f);
  GL()matrixMode(MM_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, mTex);
  GL()color4ub(mColor.r, mColor.g, mColor.b, mColor.a);
  GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL()popMatrix();
}

BlitObject* BlitObject::clone(){
  BlitObject* obj = new BlitObject(mTex, mSize, mScale, mDepth, mOffset);
  return obj;
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
  GL()disable(GL_TEXTURE_2D);
  //glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()pushMatrix();
  GL()translatef((float)mPos.x,(float)mPos.y,0.0f);
  GL()scalef((float)mSize.x,(float)mSize.y,1.0f);
  GL()color4ub(mColor.r, mColor.g, mColor.b, mColor.a);
  GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL()color4ub(255, 255, 255, 255);
  //glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);
}

ScrollBlitObject::ScrollBlitObject(int depth) : BaseBlitObject(depth, Vec2i(0,0)){
}

ScrollBlitObject::~ScrollBlitObject(){
}

void ScrollBlitObject::blit(){
  if (mDepth < 0){
    GL()pushMatrix();
    GL()translatef((float)mPos.x, (float)mPos.y, 0);
  }
  else
    GL()popMatrix();
}

void ScrollBlitObject::render(const Vec2i& pos){
  mPos = pos;
}

RenderableBlitObject::RenderableBlitObject(int width, int height, int depth) : BlitObject(width,height,depth){
  int powx = (int)(width/mScale.x);
  int powy = (int)(height/mScale.y);
  glGenRenderbuffers(1, &mRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, mRenderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)(width/mScale.x), (int)(height/mScale.y));
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glGenFramebuffers(1, &mFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTex, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderBuffer);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE){
    CGE::Log <<  "Unable to create framebuffer";
    DebugBreak();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderableBlitObject::~RenderableBlitObject(){
  glDeleteFramebuffers(1, &mFrameBuffer);
  glDeleteRenderbuffers(1, &mRenderBuffer);
}

void RenderableBlitObject::bind(){
  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
}

void RenderableBlitObject::unbind(){
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DynamicAnimation::DynamicAnimation(){

}

DynamicAnimation::~DynamicAnimation(){

}
