#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "../math/vector.h"
#include "material.h"

#define VB_POSITION 0x0001
#define VB_COLOR 0x0002
#define VB_TEXCOORD 0x0004
#define VB_NORMAL 0x0008

namespace Graphics{

typedef ::Math::Vector3D Vertex;

enum PrimitiveType{
  VB_Triangles,
  VB_Tristrip,
  VB_Lines,
  VB_Points,
};


//! a vertex buffer and optional index buffer
/*! this is the central class for managing and drawing vertex data
*/
class VertexBuffer{
public:
  VertexBuffer();
  virtual ~VertexBuffer();
  virtual void create(int type, int vertexBufferSize, int indexBufferSize);
  virtual void* lockVertexPointer()=0;
  virtual void unlockVertexPointer()=0;
  virtual short* lockIndexPointer()=0;
  virtual void unlockIndexPointer()=0;
  virtual void activate()=0;
  virtual void draw(PrimitiveType pt)=0;
  virtual void setPosition(int pos, Vertex v);
  virtual void setColor(int pos, Color c)=0;
  virtual void setTexCoord(int pos, ::Math::Vec2f t, bool dxswap=false)=0;
  virtual void setNormal(int pos, Math::Vector3D normal);
  virtual void setVertexOffset(int offset)=0;
protected:
  int structsize_;
  int vbsize_;
  int ibsize_;
  void* verts_;
  short* inds_;
  int vertoffset_;
  int coloffset_;
  int texoffset_;
  int normoffset_;
  int userVertOffset_;
};

}

#endif
