#ifndef OGLTEXTURE_H
#define OGLTEXTURE_H

#include <cstdio>

#include "../texture.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifndef QNX
#include <GL/gl.h>
#endif

namespace CGE{

class OGLTexture : public Texture {
public:
  OGLTexture(string filename);
  virtual ~OGLTexture();
  void activate() const;
  virtual void deactivate() const;
protected:
  bool load(string filename);
  GLuint tex_;
};

}

#endif
