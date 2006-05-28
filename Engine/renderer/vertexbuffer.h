#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "../math/vector.h"

#define VB_POSITION 0x0001
#define VB_COLOR 0x0002

namespace Graphics{

typedef ::Math::Vector3D Vertex;


struct Color{
  Color(){r = 255; g = 255; b = 255; a=255;}
  Color(unsigned char rn, unsigned char gn, unsigned char bn, unsigned char an){
    r = rn; g = gn; b = bn; a = an;
  }
  unsigned char r, g, b, a;
};

struct ColorVertex{
  ColorVertex();
  ColorVertex(float xn, float yn, float zn){
    x = xn; y = yn; z = zn;
    colour.r = 255; colour.g = 0; colour.b = 0; colour.a = 255;
  }
  float x, y, z;
  Color colour;
};

//! a vertex buffer and optional index buffer
/*! this is the central class for managing and drawing vertex data
*/
class VertexBuffer{
public:
  VertexBuffer();
  ~VertexBuffer();
  virtual void create(int type, int vertexBufferSize, int indexBufferSize);
  virtual void* lockVertexPointer()=0;
  virtual void unlockVertexPointer()=0;
  virtual short* lockIndexPointer()=0;
  virtual void unlockIndexPointer()=0;
  virtual void activate()=0;
  virtual void draw()=0;
  virtual void setPosition(int pos, Vertex v);
  virtual void setColor(int pos, Color c)=0;
protected:
  int structsize_;
  int vbsize_;
  int ibsize_;
  void* verts_;
  short* inds_;
  int vertoffset_;
  int coloffset_;
};

}

#endif