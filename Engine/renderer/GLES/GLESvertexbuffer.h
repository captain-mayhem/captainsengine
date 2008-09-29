#ifndef GLESVERTEXBUFFER_H
#define GLESVERTEXBUFFER_H

#include "../vertexbuffer.h"

namespace Graphics{
  
class GLESVertexBuffer : public VertexBuffer {
public:
  GLESVertexBuffer();
  ~GLESVertexBuffer();
  void create(int type, int vertexBufferSize, int indexBufferSize);
  void* lockVertexPointer();
  short* lockIndexPointer();
  void unlockVertexPointer();
  void unlockIndexPointer();
  void activate();
  void draw(PrimitiveType pt);
  void setColor(int pos, Color c);
  void setTexCoord(int pos, ::Math::Vec2f t, bool dxswap=false);
  void setVertexOffset(int offset);
protected:
  char* vb_;
  short* ib_;
  int flags_;
};

}

#endif

