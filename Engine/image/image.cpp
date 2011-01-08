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

void Image::debugWrite(const std::string& filename){
  std::string ext = mChannels == 1 ? ".pgm" : ".ppm";
  FILE* f = fopen((filename+ext).c_str(), "wb");
  fprintf(f, "P%i\n", mChannels == 1 ? 2 : 3);
  fprintf(f, "%i %i\n", mWidth, mHeight);
  fprintf(f, "255\n");
  for (int j = 0; j < mHeight; ++j){
    for (int i = 0; i < mWidth; ++i){
      for (int c = 0; c < mChannels; ++c){
        fprintf(f, "%3i ", mData[j*mWidth*mChannels+i*mChannels+c]);
      }
      fprintf(f, " ");
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

void Image::flipHorizontally(){
  int rowWidth = getRowSpan();
  unsigned char* tmp = new unsigned char[rowWidth];
  for (int i = 0; i < mHeight/2; ++i){
    memcpy(tmp, mData+i*rowWidth, rowWidth);
    memcpy(mData+i*rowWidth, mData+(mHeight-i-1)*rowWidth, rowWidth);
    memcpy(mData+(mHeight-i-1)*rowWidth, tmp, rowWidth);
  }
  delete [] tmp;
}
