#ifndef IMAGELOADER_H
#define IMAGELOADER_H

namespace CGE{

class Image;

class ImageLoader{
public:
  //! loads and returns a BMP image
  Image* loadBMP(const char *fileName);
  //! loads and returns a TGA image
  Image* loadTGA(const char *fileName);
  //! loads and returns q JGP image
  Image* loadJPG(const char *fileName);
};

}

#endif
