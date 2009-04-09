#ifndef GLESVERTEXBUFFER_H
#define GLESVERTEXBUFFER_H

#include "../vertexbuffer.h"

namespace CGE{
  
class GLESVertexBuffer : public VertexBuffer {
public:
  GLESVertexBuffer();
  ~GLESVertexBuffer();
  void create(int type, int vertexBufferSize);
  void* lockVertexPointer();
  void unlockVertexPointer();
  void activate();
  void draw(PrimitiveType pt, IndexBuffer* indices);
  void setColor(int pos, Color c);
  void setTexCoord(int pos, ::CGE::Vec2f t, bool dxswap=false);
  void setVertexOffset(int offset);
protected:
  char* vb_;
  int flags_;
};

}

#endif

