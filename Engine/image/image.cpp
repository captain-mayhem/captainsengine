#include "image.h"

#include <cstring>
#include <cstdio>
#include <algorithm>

using namespace CGE;
using std::min;

Image::Image() : mChannels(0), mWidth(0), mHeight(0), mData(NULL) {}

Image::Image(int channels, int width, int height) : mChannels(channels), mWidth(width), mHeight(height), mData(NULL){
  allocateData();
}

Image::Image(int channels, int width, int height, unsigned char* data) : mChannels(channels), mWidth(width), mHeight(height), mData(NULL){
  allocateData();
  memcpy(mData, data, getImageSize());
}

Image::Image(int channels, int width, int height, unsigned char* rgbdata, unsigned alphachannels, unsigned char* alphadata) : mChannels(channels+1), mWidth(width), mHeight(height), mData(NULL){
  allocateData();
  for (int i = 0; i < mWidth*mHeight; ++i){
    memcpy(mData+i*mChannels, rgbdata+i*channels, channels);
    mData[i*mChannels+channels] = alphadata[alphachannels*i];
  }
}

Image::Image(int channels, int width, int height, unsigned char* palette, unsigned char* indices) : mChannels(channels), mWidth(width), mHeight(height), mData(NULL){
  allocateData();
  for (int i = 0; i < mWidth*mHeight; ++i){
    memcpy(mData+i*mChannels, palette+indices[i]*mChannels, mChannels);
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

void Image::setData(unsigned char* data){
  mData = data;
}

void Image::allocateData(){
  if (mData)
    delete [] mData;
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
        if (c != 3) //skip alpha
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

void Image::crop(int x, int y, int width, int height){
	int newx = x;
	int newy = y;
	if (newx < 0)
		newx = 0;
	if (newy < 0)
		newy = 0;

	int newwidth = min(mWidth-newx, width);
	int newheight = min(mHeight-newy, height);
	if (newx == 0 && newy == 0 && newwidth == mWidth && newheight == mHeight)
		return;

	Image* ret = new Image(mChannels, newwidth, newheight);
	ret->allocateData();
	for (int j = 0; j < newheight; ++j){
		for (int i = 0; i < newwidth; ++i){
			void* to = ret->getData()+j*newwidth*mChannels+i*mChannels;
			void* from = mData+(j+newy)*mWidth*mChannels+(i+newx)*mChannels;
			memcpy(to, from, mChannels);
		}
	}
	delete [] mData;
	mData = ret->getData();
	mWidth = ret->getWidth();
	mHeight = ret->getHeight();
	ret->mData = NULL;
	delete ret;
}
