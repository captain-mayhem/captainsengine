#include "vertexbuffer.h"
#include "indexbuffer.h"

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

void VertexBuffer::setTexCoord2(int pos, CGE::Vec2f t){
  Vec2f* tex;
  tex = (Vec2f*)(((char*)verts_) + pos*structsize_ + tex2offset_);
  tex->x = t.x; tex->y = t.y;
}

void VertexBuffer::draw(PrimitiveType pt, IndexBuffer* indices){
  draw(pt, indices, 0, indices ? indices->getNumIndices() : vbsize_);
}

