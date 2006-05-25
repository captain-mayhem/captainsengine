#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#define VB_POSITION 0x0001

namespace Graphics{

struct Vertex{
  Vertex();
  Vertex(float xn, float yn, float zn){
    x = xn; y = yn; z = zn;
  }
  float x, y, z;
};

//! a vertex buffer and optional index buffer
/*! this is the central class for managing and drawing vertex data
*/
class VertexBuffer{
public:
  VertexBuffer();
  ~VertexBuffer();
  void create(int type, int structsize, int vertexBufferSize, int indexBufferSize);
  void* lockVertexPointer();
  void unlockVertexPointer();
  short* lockIndexPointer();
  void unlockIndexPointer();
  void draw();
private:
};

}

#endif