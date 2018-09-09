#ifndef OGLTEXTURE_H
#define OGLTEXTURE_H

#include <cstdio>

#include "../texture.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if !defined QNX && !defined ANDROID
#ifdef OPENGL2
#include <GL/glew.h>
#else
#include <GL/gl.h>
#endif
#else
#include <GLES2/gl2.h>
#endif

namespace CGE{

class OGLTexture : public Texture {
public:
  OGLTexture();
  virtual ~OGLTexture();
  virtual bool createFromImage(Image const* img, Format fmt);
  virtual bool createEmpty(unsigned width, unsigned height, Format fmt);
  void activate(unsigned stage=0) const;
  virtual void deactivate(unsigned stage=0) const;
  virtual Texture* copy();
  GLuint getHandle() const { return tex_; }
  static GLenum glFormat(Format fmt);
  virtual void update(int x, int y, unsigned width, unsigned height, void* data);
protected:
  GLuint tex_;
};

}

#endif