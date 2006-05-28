#include "../system/engine.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include "OGLvertexbuffer.h"

using namespace Graphics;

OGLVertexBuffer::OGLVertexBuffer(){
  vb_ = NULL;
  ib_ = NULL;
  flags_ = 0;
  verts_ = NULL;
  inds_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
}

OGLVertexBuffer::~OGLVertexBuffer(){
  SAFE_DELETE_ARRAY(vb_);
  SAFE_DELETE_ARRAY(ib_);
}

#include "../system/engine.h"

void OGLVertexBuffer::create(int type, int vertexBufferSize, int indexBufferSize){
  vbsize_ = vertexBufferSize;
  ibsize_ = indexBufferSize;
  flags_ = type;
  int offset = 0;
  if (flags_ & VB_POSITION){
    vertoffset_ = offset;
    offset += 3*sizeof(float);
  }
  if (flags_ & VB_COLOR){
    coloffset_ = offset;
    offset += 4*sizeof(char);
  }
  structsize_ = offset;
  
  vb_ = new char[vertexBufferSize*structsize_];
  if (indexBufferSize == 0)
    return;
  ib_ = new short[indexBufferSize*sizeof(short)];
}

void* OGLVertexBuffer::lockVertexPointer(){
  verts_ = (void*)vb_;
  return verts_;
}

short* OGLVertexBuffer::lockIndexPointer(){
  inds_ = ib_;
  return inds_;
}

void OGLVertexBuffer::unlockVertexPointer(){
  verts_ = NULL;
}

void OGLVertexBuffer::unlockIndexPointer(){
  inds_ = NULL;
}

void OGLVertexBuffer::activate(){
  if (flags_ & VB_POSITION){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, structsize_, vb_+vertoffset_);
  }
  else
    glDisableClientState(GL_VERTEX_ARRAY);
  if (flags_ & VB_COLOR){
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, structsize_, vb_+coloffset_);
  }
  else
    glDisableClientState(GL_COLOR_ARRAY);
}

void OGLVertexBuffer::draw(){
  glDrawElements(GL_TRIANGLES, ibsize_, GL_UNSIGNED_SHORT, ib_);
}

void OGLVertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.r; col->g = c.g; col->b = c.b; col->a = c.a;
}