#include "../../system/engine.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include "OGLvertexbuffer.h"
#include "OGLindexbuffer.h"

using namespace CGE;

OGLVertexBuffer::OGLVertexBuffer(){
  vb_ = NULL;
  flags_ = 0;
  verts_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
  texoffset_ = -1;
  normoffset_ = -1;
  userVertOffset_ = 0;
}

OGLVertexBuffer::~OGLVertexBuffer(){
  SAFE_DELETE_ARRAY(vb_);
}

void OGLVertexBuffer::create(int type, int vertexBufferSize){
  vbsize_ = vertexBufferSize;
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
  if (flags_ & VB_TEXCOORD){
    texoffset_ = offset;
    offset += 2*sizeof(float);
  }
  if (flags_ & VB_NORMAL){
    normoffset_ = offset;
    offset += 3*sizeof(float);
  }
  structsize_ = offset;
  
  vb_ = new char[vertexBufferSize*structsize_];
}

void* OGLVertexBuffer::lockVertexPointer(){
  verts_ = (void*)vb_;
  return verts_;
}

void OGLVertexBuffer::unlockVertexPointer(){
  verts_ = NULL;
}

void OGLVertexBuffer::activate(){
  if (flags_ & VB_POSITION){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, structsize_, vb_+vertoffset_+userVertOffset_);
  }
  else
    glDisableClientState(GL_VERTEX_ARRAY);
  if (flags_ & VB_COLOR){
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, structsize_, vb_+coloffset_+userVertOffset_);
  }
  else
    glDisableClientState(GL_COLOR_ARRAY);
  if (flags_ & VB_TEXCOORD){
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, structsize_, vb_+texoffset_+userVertOffset_);
  }
  else
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if (flags_ & VB_NORMAL){
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, structsize_, vb_+normoffset_+userVertOffset_);
  }
  else
    glDisableClientState(GL_NORMAL_ARRAY);
}

void OGLVertexBuffer::draw(PrimitiveType pt, IndexBuffer* indices){
  if (indices == NULL){
    if (pt == VB_Tristrip)
      glDrawArrays(GL_TRIANGLE_STRIP, 0, vbsize_);
    else if (pt == VB_Triangles)
      glDrawArrays(GL_TRIANGLES, 0, vbsize_);
    else if (pt == VB_Trifan)
      glDrawArrays(GL_TRIANGLE_FAN, 0, vbsize_);
    else if (pt == VB_Lines)
      glDrawArrays(GL_LINES, 0, vbsize_);
    else if (pt == VB_Points)
      glDrawArrays(GL_POINTS, 0, vbsize_);
    return;
  }
  OGLIndexBuffer* oglidx = (OGLIndexBuffer*)indices;
  if (pt == VB_Tristrip)
    glDrawElements(GL_TRIANGLE_STRIP, oglidx->getNumIndices(), oglidx->getType(), oglidx->getIndices());
  else if (pt == VB_Triangles)
    glDrawElements(GL_TRIANGLES, oglidx->getNumIndices(), oglidx->getType(), oglidx->getIndices());
  else if (pt == VB_Trifan)
    glDrawElements(GL_TRIANGLE_FAN, oglidx->getNumIndices(), oglidx->getType(), oglidx->getIndices());
  else if (pt == VB_Lines)
    glDrawElements(GL_LINES, oglidx->getNumIndices(), oglidx->getType(), oglidx->getIndices());
  else if (pt == VB_Points)
    glDrawElements(GL_POINTS, oglidx->getNumIndices(), oglidx->getType(), oglidx->getIndices());
}

void OGLVertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.r; col->g = c.g; col->b = c.b; col->a = c.a;
}

void OGLVertexBuffer::setTexCoord(int pos, ::CGE::Vec2f t, bool dxswap){
  ::CGE::Vec2f* tex;
  tex = (::CGE::Vec2f*)(((char*)verts_)+pos*structsize_+texoffset_);
  tex->x = t.x; tex->y = t.y;
}

void OGLVertexBuffer::setVertexOffset(int offset){
  userVertOffset_ = 4*offset*structsize_;//offset*structsize_;
  glVertexPointer(3, GL_FLOAT, structsize_, vb_+vertoffset_+userVertOffset_);
  glColorPointer(4, GL_UNSIGNED_BYTE, structsize_, vb_+coloffset_+userVertOffset_);
  glTexCoordPointer(2, GL_FLOAT, structsize_, vb_+texoffset_+userVertOffset_);
  glNormalPointer(GL_FLOAT, structsize_, vb_+normoffset_+userVertOffset_);
}
