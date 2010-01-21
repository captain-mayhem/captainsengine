#include "BlitObjects.h"
#include <wx/image.h>
#include "Engine.h"

BaseBlitObject::BaseBlitObject(int depth, const Vec2i& size) : 
mPos(), mSize(size), mDepth(depth){

}

BaseBlitObject::~BaseBlitObject(){

}

BlitObject::BlitObject(int width, int height) : BaseBlitObject(0, Vec2i(width, height)), mOffset(){
  Vec2i pow2(Engine::roundToPowerOf2(mSize.x), Engine::roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  mMirrorX = false;
  mDeleteTex = true;
  glGenTextures(1, &mTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
BaseBlitObject(depth, Vec2i()), mOffset(offset){
  wxImage image = Engine::instance()->getImage(texture);
  mTex = Engine::instance()->genTexture(image, mSize, mScale);
  mMirrorX = false;
  mDeleteTex = true;
}

BlitObject::BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
BaseBlitObject(depth, size), mOffset(offset), mScale(scale), mTex(texture)
{
  mMirrorX = false;
  mDeleteTex = false;
}

BlitObject::~BlitObject(){
  if (mDeleteTex)
    glDeleteTextures(1, &mTex);
}

bool BlitObject::operator<(const BlitObject& obj){
  return mDepth < obj.mDepth;
}

void BlitObject::render(Vec2i pos, bool mirrorx){
  mPos.x = mOffset.x+pos.x;
  mPos.y = mOffset.y+pos.y;
  mMirrorX = mirrorx;
  Engine::instance()->insertToBlit(this);
}

void BlitObject::blit(){
  glPushMatrix();
  //if (mMirrorX)
  //  glTranslatef(mPos.x+mOffset.x,mPos.y,0.0f);
  //else
  glTranslatef(mPos.x,mPos.y,0.0f);
  glScalef(mSize.x,mSize.y,1.0f);
  if (mMirrorX)
    glScalef(-1.,1.,1.);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(mScale.x, mScale.y, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glPopMatrix();
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
  glDisable(GL_TEXTURE_2D);
  //glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  glPushMatrix();
  glTranslatef(mPos.x,mPos.y,0.0f);
  glScalef(mSize.x,mSize.y,1.0f);
  glColor4ub(mColor.r, mColor.g, mColor.b, mColor.a);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glColor4ub(255, 255, 255, 255);
  //glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPopMatrix();
  glEnable(GL_TEXTURE_2D);
}

ScrollBlitObject::ScrollBlitObject(int depth) : BaseBlitObject(depth, Vec2i(0,0)){
}

ScrollBlitObject::~ScrollBlitObject(){
}

void ScrollBlitObject::blit(){
  if (mDepth < 0){
    glPushMatrix();
    glTranslatef(mPos.x, mPos.y, 0);
  }
  else
    glPopMatrix();
}

void ScrollBlitObject::render(const Vec2i& pos){
  mPos = pos;
}

RenderableBlitObject::RenderableBlitObject(int width, int height) : BlitObject(width,height){
  int powx = (int)(width/mScale.x);
  int powy = (int)(height/mScale.y);
  glGenRenderbuffersEXT(1, &mRenderBuffer);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRenderBuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, (int)(width/mScale.x), (int)(height/mScale.y));
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

  glGenFramebuffersEXT(1, &mFrameBuffer);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBuffer);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mTex, 0);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRenderBuffer);
  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if (status != GL_FRAMEBUFFER_COMPLETE_EXT){
    assert(false && "Unable to create framebuffer");
  }
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

RenderableBlitObject::~RenderableBlitObject(){
  glDeleteFramebuffersEXT(1, &mFrameBuffer);
  glDeleteRenderbuffersEXT(1, &mRenderBuffer);
}

void RenderableBlitObject::bind(){
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBuffer);
}

void RenderableBlitObject::unbind(){
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
