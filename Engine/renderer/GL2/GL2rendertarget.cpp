#include "GL2rendertarget.h"
#include "GL2texture.h"
#include <io/Tracing.h>

using namespace CGE;

TR_CHANNEL(CGE_Rendertarget);

GL2RenderTarget::GL2RenderTarget() : mWidth(0), mHeight(0), mCTCount(0){
  GLint fb;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
  mFBO = fb;
}

GL2RenderTarget::GL2RenderTarget(unsigned width, unsigned height) : mWidth(width), mHeight(height), mCTCount(0){
  glGenFramebuffers(1, &mFBO);
}

GL2RenderTarget::~GL2RenderTarget(){
  for (unsigned i = 0; i < mTextures.size(); ++i){
    delete mTextures[i];
  }
  for (unsigned i = 0; i < mRenderbuffers.size(); ++i){
    glDeleteRenderbuffers(1, &mRenderbuffers[i]);
  }
  glDeleteFramebuffers(1, &mFBO);
}

void GL2RenderTarget::bind(){
  glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
  if (mCTCount > 1){
    std::vector<GLenum> buffers;
    buffers.resize(mCTCount);
    for (unsigned i = 0; i < mCTCount; ++i){
      buffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }
    glDrawBuffers(mCTCount, buffers.data());
  }
}

void GL2RenderTarget::addTexture(Texture::Format fmt){
  TR_USE(CGE_Rendertarget);
  GL2Texture* tex = new GL2Texture();
  if (!tex->createEmpty(mWidth, mHeight, fmt)){
    TR_ERROR("cannot create texture of format %i", fmt);
    return;
  }
  if (fmt != Texture::DEPTH)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+mCTCount++, GL_TEXTURE_2D, tex->getHandle(), 0);
  else
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->getHandle(), 0);
  mTextures.push_back(tex);
}

void GL2RenderTarget::addRenderbuffer(Texture::Format fmt){
  GLuint renderBuffer;
  glGenRenderbuffers(1, &renderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL2Texture::glFormat(fmt), mWidth, mHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  GLenum attach;
  if (fmt == Texture::DEPTH)
    attach = GL_DEPTH_ATTACHMENT;
  else
    attach = GL_COLOR_ATTACHMENT0;

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, renderBuffer);

  mRenderbuffers.push_back(renderBuffer);
}

bool GL2RenderTarget::isComplete(){
  TR_USE(CGE_Rendertarget);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE){
    char const* statstr = "UNKNOWN";
    switch (status){
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      statstr = "Incomplete attachment";
      break;
    default:
      break;
    }
    TR_ERROR("Unable to create framebuffer - status %s (%i)", statstr, status);
    return false;
  }
  return true;
}

Texture* GL2RenderTarget::getTexture(unsigned idx){
  return mTextures[idx];
}
