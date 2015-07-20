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

void VertexBuffer::setNormal(int pos, Math::Vector3D normal){
  Math::Vector3D* norm;
  norm = (Math::Vector3D*)(((char*)verts_)+pos*structsize_+normoffset_);
  norm->x = normal.x; norm->y = normal.y; norm->z = normal.z; 
}

