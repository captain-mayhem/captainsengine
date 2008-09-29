#ifndef OGLTEXTURE_H
#define OGLTEXTURE_H

#include <cstdio>

#ifdef WIN32
#undef FAR
extern "C"{
#include "../include/jpeglib.h"
}
#endif
#ifdef UNIX
#include "../include/jpeglib.h"
#endif

#include "../texture.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

namespace Graphics{

// tga file types
#define TGA_RGB		 2
#define TGA_A		 3
#define TGA_RLE		10

//! image structure for holding one loaded image
struct Image{
  //! Number of channels
  //! 3 means RGB, 4 is RGBA
  int channels;
  //! image width
  int sizeX;
  //! image height
  int sizeY;
  //! pixel data
  unsigned char *data;
};

class OGLTexture : public Texture {
public:
  OGLTexture(string filename);
  virtual ~OGLTexture(){}
  void activate();
  virtual void deactivate();
protected:
  bool load(string filename);
  //! loads and returns a BMP image
  Image *loadBMP(const char *fileName);
  //! loads and returns a TGA image
  Image *loadTGA(const char *fileName);
  //! loads and returns q JGP image
  Image *loadJPG(const char *fileName);
  //! This decompresses the JPEG and fills in the image data
  void decodeJPG(jpeg_decompress_struct* cinfo, Image *pImageData);
  GLuint tex_;
};

}

#endif
