#include "vertexbuffer.h"

using namespace CGE;

VertexBuffer::VertexBuffer(){

}

VertexBuffer::~VertexBuffer(){

}

void VertexBuffer::setPosition(int pos, Vec3f v){
  Vec3f* vert;
  vert = (Vec3f*)(((char*)verts_)+pos*structsize_+vertoffset_);
  vert->x = v.x; vert->y = v.y; vert->z = v.z;
}

void VertexBuffer::setNormal(int pos, CGE::Vector3D normal){
  CGE::Vector3D* norm;
  norm = (CGE::Vector3D*)(((char*)verts_)+pos*structsize_+normoffset_);
  norm->x = normal.x; norm->y = normal.y; norm->z = normal.z; 
}

