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

bool OGLTexture::loadFromFile(std::string const & filename){
  filename_ = filename;
  TR_USE(CGE_Texture_OGL);
  Image *img = NULL;

  img = ImageLoader::load(filename.c_str());

  if (!img)
    return false;

  glBindTexture(GL_TEXTURE_2D, tex_);
  glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->getWidth(), img->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, img->getData());

  delete img;
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

void OGLTexture::activate() const{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex_);
}


void OGLTexture::deactivate() const{
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
    return GL_DEPTH_COMPONENT16;
  }
  return GL_INVALID_ENUM;
}
