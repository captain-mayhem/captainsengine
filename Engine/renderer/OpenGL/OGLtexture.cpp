#include "../../system/engine.h"
#include "OGLtexture.h"
#include "../../image/image.h"
#include "../../image/loader.h"

using namespace CGE;

typedef unsigned char byte;
typedef unsigned short WORD;

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

TR_CHANNEL(CGE_Texture_OGL);

OGLTexture::OGLTexture() : Texture(), tex_(0){
  glGenTextures(1, &tex_);
}

OGLTexture::~OGLTexture(){
  glDeleteTextures(1, &tex_);
}

bool OGLTexture::createFromImage(Image* img, Format fmt){
  TR_USE(CGE_Texture_OGL);
  if (fmt == AUTO)
    fmt = (Format)img->getNumChannels();

  glBindTexture(GL_TEXTURE_2D, tex_);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);

  GLenum glfmt = glFormat(fmt);
  glTexImage2D(GL_TEXTURE_2D, 0, glfmt, img->getWidth(), img->getHeight(), 0, glfmt, GL_UNSIGNED_BYTE, img->getData());

  return true;
}

bool OGLTexture::createEmpty(unsigned width, unsigned height, Format fmt){
  if (fmt == AUTO)
    return false;

  glBindTexture(GL_TEXTURE_2D, tex_);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  GLenum glformat = glFormat(fmt);
  glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, GL_UNSIGNED_BYTE, NULL);
  return true;
}

void OGLTexture::activate(unsigned stage) const{
  glActiveTexture(GL_TEXTURE0 + stage);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex_);
}


void OGLTexture::deactivate(unsigned stage) const{
  glActiveTexture(GL_TEXTURE0 + stage);
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

GLenum OGLTexture::glFormat(Format fmt){
  switch (fmt){
  case GRAY:
    return GL_LUMINANCE;
  case RGB:
    return GL_RGB;
  case RGBA:
    return GL_RGBA;
  case DEPTH:
#ifdef RENDER_EMBEDDED
    return GL_DEPTH_COMPONENT16;
#else
    return GL_DEPTH_COMPONENT;
#endif
  case ALPHA:
    return GL_ALPHA;
  }
  return GL_INVALID_ENUM;
}
