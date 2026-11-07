#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <cge_export.h>

#include <string>

namespace CGE{

class Image;

class CGE_API ImageLoader{
public:
  enum Type{
    UNKNOWN,
    JPG,
    GIF,
    BMP,
    PNG,
    TIFF,
  };
  static Image* load(void* memory, unsigned size, Type t);
  //! loads an image asset (see Assets)
  static Image* load(const char* filename, Type t = UNKNOWN);
  static Type determineType(const std::string& filename);
protected:
  //! decode an image in memory
  static Image* loadBMP(void* memory, unsigned size);
  static Image* loadJPG(void* memory, unsigned size);
  static Image* loadGIF(void* memory, unsigned size);
  static Image* loadPNG(void* memory, unsigned size);
  //! the first image of a TIFF file (tiff.cpp says which kinds)
  static Image* loadTIFF(void* memory, unsigned size);
};

}

#endif
