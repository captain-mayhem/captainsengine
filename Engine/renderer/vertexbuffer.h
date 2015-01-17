#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "../math/vector.h"
#include "material.h"

#define VB_POSITION 0x0001
#define VB_COLOR 0x0002
#define VB_TEXCOORD 0x0004
#define VB_NORMAL 0x0008

namespace CGE{

class IndexBuffer;

enum PrimitiveType{
  VB_Triangles,
  VB_Tristrip,
  VB_Trifan,
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
  virtual void create(int type, int vertexBufferSize)=0;
  virtual void* lockVertexPointer()=0;
  virtual void unlockVertexPointer()=0;
  virtual void activate()=0;
  virtual void draw(PrimitiveType pt, IndexBuffer* indices, int offset, int count)=0;
  void draw(PrimitiveType pt, IndexBuffer* indices);
  virtual void setPosition(int pos, Vec3f v);
  virtual void setColor(int pos, Color c)=0;
  virtual void setTexCoord(int pos, CGE::Vec2f t)=0;
  virtual void setNormal(int pos, CGE::Vec3f normal);
  virtual void setVertexOffset(int offset)=0;
protected:
  int structsize_;
  int vbsize_;
  void* verts_;
  int vertoffset_;
  int coloffset_;
  int texoffset_;
  int normoffset_;
  int userVertOffset_;
};

}

#endif
