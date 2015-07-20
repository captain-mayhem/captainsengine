//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: texture.hh                                                 |
//  ==================================================================

#ifndef TEXTURE_HH
#define TEXTURE_HH

#ifdef WIN32
#include <windows.h>
#include "jpeglib_win.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include "jpeglib.h"
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>


// tga file types
#define TGA_RGB		 2
#define TGA_A		 3
#define TGA_RLE		10

//! image structure for holding one loaded image
struct Image
{
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

//! loads and manages textures
/*! this class is responsible for loading texture images and maintaining
 * the arrays for access to the textures.
 */
class Texture{
public:
  //! which format the image has
  enum format{
    bmp,
    jpg,
    tga
  };
  //! what texture type should be loaded
  enum type{
    floor,
    wall,
    font,
    hero,
    monster,
		furniture,
    overlay,
    other
  };
  //! Constructor
  Texture();
  //! Destructor
  ~Texture();
  //! load and prepare all textures for use with OpenGL.
  int loadGLTextures();
  //! Creates one texture
  /*! \param f the image file format
   *  \param t what type of texture should be loaded
   *  \param strFileName the filename of the image
   *  \param textureID the id of the texture (the place in the array)
   */
  void createTexture(enum format f, enum type t, const char* strFileName, int textureID);
  //! Create a texture
  /*! Creates a texture and stores the place in storage instead of one of the texture class arrays
   */
  void createTexture(enum format f, unsigned int *storage, const char* strFileName, int textureID);

  //! floor textures
  GLuint* floorTex;
  //! wall and door textures
  GLuint* wallTex;
  //! font texture(s)
  GLuint fontTex[1];
  //! hero textures
  GLuint* heroTex;
  //! monster textures
  GLuint* monsterTex;
	//! furniture textures
	GLuint* furnitureTex;
  //! overlay textures
  GLuint* overlayTex;
  //! some other misc textures
  GLuint* otherTex;
  
private:
  //! loads and returns a BMP image
  Image *loadBMP(const char *fileName);
  //! loads and returns a TGA image
  Image *loadTGA(const char *fileName);
  //! loads and returns q JGP image
  Image *loadJPG(const char *fileName);
  //! This decompresses the JPEG and fills in the image data
  void decodeJPG(jpeg_decompress_struct* cinfo, Image *pImageData); 
};

extern class Texture tex;

#endif
