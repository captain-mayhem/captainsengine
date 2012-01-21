#include "BlitObjects.h"

#include <system/engine.h>

#include "Engine.h"

TR_CHANNEL(ADV_Render_BlitObject);

BaseBlitObject::BaseBlitObject(int depth, const Vec2i& size) : 
mPos(), mSize(size), mDepth(depth){

}

BaseBlitObject::~BaseBlitObject(){

}

BlitObject::BlitObject(int width, int height, int depth, GLenum format) : 
BaseBlitObject(depth, Vec2i(width, height)), mOffset(), mMirrorOffset(), mRotAngle(0.0f), mBlendAdditive(false){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
  glGenTextures(1, &mTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, format, pow2.x, pow2.y, 0, format, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
BaseBlitObject(depth, Vec2i()), mOffset(offset), mMirrorOffset(), mRotAngle(0.0f), mBlendAdditive(false){
  CGE::Image* image = Engine::instance()->getImage(texture);
  mTex = Engine::instance()->genTexture(image, mSize, mScale);
  delete image;
  mZoomScale = Vec2f(1.0f,1.0f);
  mDeleteTex = true;
}

BlitObject::BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
BaseBlitObject(depth, size), mOffset(offset), mScale(scale), mTex(texture), mMirrorOffset(), mRotAngle(0), mBlendAdditive(false)
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
  if (mBlendAdditive)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
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
  if (mBlendAdditive)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

BlitObject* BlitObject::clone(){
  BlitObject* obj = new BlitObject(mTex, mSize, mScale, mDepth, mOffset);
  return obj;
}

void BlitObject::updateTexture(unsigned width, unsigned height, void* data){
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
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
  TR_USE(ADV_Render_BlitObject);
  int powx = (int)(width/mScale.x);
  int powy = (int)(height/mScale.y);
  glGenRenderbuffers(1, &mRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, mRenderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, (int)(width/mScale.x), (int)(height/mScale.y));
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glGenFramebuffers(1, &mFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTex, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderBuffer);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE){
    TR_ERROR("Unable to create framebuffer - status %i", status);
    //DebugBreak();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderableBlitObject::~RenderableBlitObject(){
  glDeleteFramebuffers(1, &mFrameBuffer);
  glDeleteRenderbuffers(1, &mRenderBuffer);
}

void RenderableBlitObject::bind(){
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
}

void RenderableBlitObject::unbind(){
  glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer);
}

DynamicAnimation::DynamicAnimation(){

}

DynamicAnimation::~DynamicAnimation(){

}

MirrorObject::MirrorObject(int width, int height, int depth, unsigned char strength) : RenderableBlitObject(width, height, depth), mOpacity(strength){
}

bool MirrorObject::update(unsigned interval){
  bind();
  GL()disable(GL_TEXTURE_2D);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()color4ub(255, 255, 0, 128);
  //GL()matrixMode(MM_MODELVIEW);
  GL()pushMatrix();
  GL()loadIdentity();
  GL()translatef(0.0f, (float)Engine::instance()->getResolution().y, 0.0f);
  GL()scalef(1.0f,-1.0f,1.0f);
  GL()pushMatrix();
  GL()translatef((float)mRoom->getScrollOffset().x, (float)mRoom->getScrollOffset().y, 0.0f);
  //GL()loadIdentity();
  GL()vertexPointer(3, GL_FLOAT, 0, mPolygon);
  GL()drawArrays(GL_TRIANGLE_FAN, 0, 4);
  GL()popMatrix();
  //GL()matrixMode(MM_PROJECTION);
  //GL()popMatrix();
  //GL()matrixMode(MM_MODELVIEW);
  glDepthMask(GL_FALSE);
  glDepthFunc(GL_GEQUAL);
  glEnable(GL_DEPTH_TEST);
  GL()enable(GL_TEXTURE_2D);
  Engine::instance()->beginRendering();
  CharacterObject* self = Engine::instance()->getCharacter("self");
  if (self)
    renderCharacter(self);
  //mRoom->getCharacters();
  Engine::instance()->endRendering();
  glDepthMask(GL_TRUE);
  glDisable(GL_DEPTH_TEST);

  /*glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()disable(GL_TEXTURE_2D);
  GL()color4ub(255, 255, 255, 0);
  GL()translatef((float)mRoom->getScrollOffset().x, (float)mRoom->getScrollOffset().y, 0.0f);
  GL()vertexPointer(2, GL_FLOAT, 0, mPolygon);
  GL()drawArrays(GL_TRIANGLE_FAN, 0, 4);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);*/
  //multiply alpha
  //GL()pushMatrix();
  //GL()loadIdentity();
  GL()scalef((float)mSize.x,(float)mSize.y,1.0f);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  GL()color4ub(255, 255, 255, mOpacity);
  GL()disable(GL_TEXTURE_2D);
  GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL()popMatrix();
  GL()enable(GL_TEXTURE_2D);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Engine::instance()->restoreRenderDefaults();
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
    mPolygon[3*i] = (float)points[i].x;
    mPolygon[3*i+1] = (float)points[i].y;
    mPolygon[3*i+2] = -1.0f;
  }
  mMirrorCenter = (xmin+xmax)/2;
}

void MirrorObject::renderCharacter(CharacterObject* chr){
  Vec2i oldpos = chr->getPosition();
  Vec2i newpos = oldpos;
  int mirrorbase = mDepth*Engine::instance()->getWalkGridSize();
  newpos.y = mirrorbase + mMirrorOffset.y;
  int diff = newpos.y-oldpos.y;
  newpos.y += diff;
  if (mPositionDependent){
    int xdiff = mMirrorCenter-newpos.x;
    int diffadapt = mDepth-chr->getDepth();
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
}

void MirrorObject::setWallMirror(Vec2i offset, bool positionDependent){
  mMirrorOffset = offset;
  mPositionDependent = positionDependent;
}
