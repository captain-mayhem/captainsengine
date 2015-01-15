#ifndef OGLVERTEXBUFFER_H
#define OGLVERTEXBUFFER_H

#include "../vertexbuffer.h"

namespace CGE{
  
class OGLVertexBuffer : public VertexBuffer {
public:
  OGLVertexBuffer();
  ~OGLVertexBuffer();
  void create(int type, int vertexBufferSize);
  void createIndexBuffer(short indexNum, short indexBufferSize);
  void* lockVertexPointer();
  void unlockVertexPointer();
  void activate();
  void draw(PrimitiveType pt, IndexBuffer* indices, int offset, int count);
  void setColor(int pos, Color c);
  void setTexCoord(int pos, ::CGE::Vec2f t, bool dxswap=false);
  void setVertexOffset(int offset);
protected:
  char* vb_;
  int flags_;
};

}

#endif

