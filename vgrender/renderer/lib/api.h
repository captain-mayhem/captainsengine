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

class Triangle{
public:
  Triangle(float x1, float y1, float x2, float y2, float x3, float y3){
    this->x0 = x1; this->y0 = y1;
    this->x1 = x2; this->y1 = y2;
    this->x2 = x3; this->y2 = y3;
  }
  void raster(VRSurface* surface);
protected:
  float f01(float x, float y){return (y0 - y1)*x + (x1 - x0)*y + x0*y1 - x1*y0;}
  float f12(float x, float y){return (y1 - y2)*x + (x2 - x1)*y + x1*y2 - x2*y1;}
  float f20(float x, float y){return (y2 - y0)*x + (x0 - x2)*y + x2*y0 - x0*y2;}
  float x0; float y0;
  float x1; float y1;
  float x2; float y2;
};
