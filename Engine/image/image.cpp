#include "image.h"

#include <cstring>

using namespace CGE;

Image::Image() : mChannels(0), mWidth(0), mHeight(0), mData(NULL) {}

Image::Image(int channels, int width, int height, unsigned char* data) : mChannels(channels), mWidth(width), mHeight(height){
  allocateData();
  memcpy(mData, data, getImageSize());
}

Image::Image(int channels, int width, int height, unsigned char* rgbdata, unsigned alphachannels, unsigned char* alphadata) : mChannels(channels+1), mWidth(width), mHeight(height){
  allocateData();
  for (int i = 0; i < mWidth*mHeight; ++i){
    memcpy(mData+i*mChannels, rgbdata+i*channels, channels);
    mData[i*mChannels+channels] = alphadata[alphachannels*i];
  }
}

Image::~Image(){
  delete [] mData;
}

void Image::setFormat(int channels, int width, int height){
  mChannels = channels;
  mWidth = width;
  mHeight = height;
}

void Image::allocateData(){
  mData = new unsigned char[sizeof(unsigned char)*getRowSpan()*mHeight];
}

unsigned char Image::getPixelChannel(int x, int y, int channel) const{
  return mData[y*getRowSpan()+x*mChannels+channel];
}
