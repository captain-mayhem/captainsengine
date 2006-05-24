#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#define VB_POSITION 0x0001

namespace Graphics{
  
class VertexBuffer{
public:
  VertexBuffer();
  ~VertexBuffer();
  void create(int type, int vertexBufferSize, int indexBufferSize);
  void* getVertexPointer();
  void* getIndexPointer();
  void draw();
private:
};

}

#endif