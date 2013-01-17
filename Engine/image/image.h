#ifndef CGE_IMAGE_H
#define CGE_IMAGE_H

#include <string>

namespace CGE{
  class Image{
  public:
    Image();
    Image(int channels, int width, int height);
    Image(int channels, int width, int height, unsigned char* data);
    Image(int channels, int width, int height, unsigned char* rgbdata, unsigned alphachannels, unsigned char* alphadata);
    Image(int channels, int width, int height, unsigned char* palette, unsigned char* indices);
    ~Image();
    void setFormat(int channels, int width, int height);
    void allocateData();
    unsigned getWidth() const {return mWidth;}
    unsigned getHeight() const {return mHeight;}
    unsigned getRowSpan() const {return mWidth*mChannels;}
    unsigned getImageSize() const {return mWidth*mChannels*mHeight;}
    unsigned char* getData() const {return mData;}
    unsigned char getPixelChannel(int x, int y, int channel) const;
    bool hasAlpha() const {return mChannels == 4;}
    unsigned getNumChannels() const {return mChannels;}
    void debugWrite(const std::string& filename);
    void flipHorizontally();
  protected:
    //! Number of channels
    //! 3 means RGB, 4 is RGBA
    int mChannels;
    //! image width
    int mWidth;
    //! image height
    int mHeight;
    //! pixel data
    unsigned char* mData;
  };
}

#endif
