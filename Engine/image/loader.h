#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <string>

namespace CGE{

class Image;

class ImageLoader{
public:
  enum Type{
    UNKNOWN,
    JPG,
    GIF,
  };
  static Image* load(void* memory, unsigned size, Type t);
  static Image* load(const char* filename, Type t);
  static Type determineType(std::string filename);
protected:
  //! loads and returns a BMP image
  static Image* loadBMP(const char *fileName);
  //! loads and returns a TGA image
  static Image* loadTGA(const char *fileName);
  //! loads and returns a JGP image
  static Image* loadJPG(const char *fileName);
  static Image* loadJPG(void* memory, unsigned size);
   //! loads and returns a GIF image
  static Image* loadGIF(void* memory, unsigned size);
};

}

#endif
