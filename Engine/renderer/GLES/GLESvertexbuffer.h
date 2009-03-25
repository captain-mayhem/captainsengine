#ifndef GLESVERTEXBUFFER_H
#define GLESVERTEXBUFFER_H

#include "../vertexbuffer.h"

namespace Graphics{
  
class GLESVertexBuffer : public VertexBuffer {
public:
  GLESVertexBuffer();
  ~GLESVertexBuffer();
  void create(int type, int vertexBufferSize, short numIndexBuffers);
  void createIndexBuffer(short indexNum, short indexBufferSize);
  void* lockVertexPointer();
  short* lockIndexPointer(short indexNum);
  void unlockVertexPointer();
  void unlockIndexPointer(short indexNum);
  void activate();
  void draw(PrimitiveType pt, short indexNum);
  void setColor(int pos, Color c);
  void setTexCoord(int pos, ::CGE::Vec2f t, bool dxswap=false);
  void setVertexOffset(int offset);
protected:
  char* vb_;
  short** ibs_;
  int flags_;
};

}

#endif

