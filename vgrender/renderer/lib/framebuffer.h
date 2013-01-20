#ifndef VR_FRAMEBUFFER_H
#define VR_FRAMEBUFFER_H

#include <image/image.h>

class VRSurface{
public:
  VRSurface(int width, int height);
  void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a){
    int offset = y*mBuffer.getRowSpan()+x*mBuffer.getNumChannels();
    unsigned char* addr = mBuffer.getData()+offset;
    addr[0] = r; addr[1] = g; addr[2] = b; addr[3] = a;
  }
  void clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
  void debugWrite(const std::string& filename) {mBuffer.debugWrite(filename);}
protected:
  CGE::Image mBuffer;
};

#endif
