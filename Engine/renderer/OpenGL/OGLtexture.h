#ifndef OGLTEXTURE_H
#define OGLTEXTURE_H

#include <cstdio>

#include "../texture.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifndef QNX
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
  virtual bool createFromImage(Image* img, Format fmt);
  virtual bool createEmpty(unsigned width, unsigned height, Format fmt);
  void activate() const;
  virtual void deactivate() const;
  GLuint getHandle() { return tex_; }
  static GLenum glFormat(Format fmt);
protected:
  GLuint tex_;
};

}

#endif
