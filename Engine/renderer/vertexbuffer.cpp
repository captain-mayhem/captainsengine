#include "vertexbuffer.h"

using namespace Graphics;

VertexBuffer::VertexBuffer(){

}

VertexBuffer::~VertexBuffer(){

}

void VertexBuffer::create(int type, int vertexBufferSize, int indexBufferSize){

}

void VertexBuffer::setPosition(int pos, Vertex v){
  Vertex* vert;
  vert = (Vertex*)(((char*)verts_)+pos*structsize_+vertoffset_);
  vert->x = v.x; vert->y = v.y; vert->z = v.z;
}
