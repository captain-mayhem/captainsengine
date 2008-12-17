#include "../../system/engine.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GLES/gl.h>
#include "GLESvertexbuffer.h"

using namespace Graphics;

GLESVertexBuffer::GLESVertexBuffer(){
  vb_ = NULL;
  ib_ = NULL;
  flags_ = 0;
  verts_ = NULL;
  inds_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
  texoffset_ = -1;
  normoffset_ = -1;
  userVertOffset_ = 0;
}

GLESVertexBuffer::~GLESVertexBuffer(){
  SAFE_DELETE_ARRAY(vb_);
  SAFE_DELETE_ARRAY(ib_);
}

#include "../../system/engine.h"

void GLESVertexBuffer::create(int type, int vertexBufferSize, int indexBufferSize){
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
  if (indexBufferSize == 0){
    ib_ = NULL;
    return;
  }
  ib_ = new short[indexBufferSize];
}

void* GLESVertexBuffer::lockVertexPointer(){
  verts_ = (void*)vb_;
  return verts_;
}

short* GLESVertexBuffer::lockIndexPointer(){
  inds_ = ib_;
  return inds_;
}

void GLESVertexBuffer::unlockVertexPointer(){
  verts_ = NULL;
}

void GLESVertexBuffer::unlockIndexPointer(){
  inds_ = NULL;
}

void GLESVertexBuffer::activate(){
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

void GLESVertexBuffer::draw(PrimitiveType pt){
  if (ibsize_ == 0){
    if (pt == VB_Tristrip)
      glDrawArrays(GL_TRIANGLE_STRIP, 0, vbsize_);
    else if (pt == VB_Triangles)
      glDrawArrays(GL_TRIANGLES, 0, vbsize_);
    else if (pt == VB_Lines)
      glDrawArrays(GL_LINES, 0, vbsize_);
    return;
  }
  if (pt == VB_Tristrip)
    glDrawElements(GL_TRIANGLE_STRIP, ibsize_, GL_UNSIGNED_SHORT, ib_);
  else if (pt == VB_Triangles)
    glDrawElements(GL_TRIANGLES, ibsize_, GL_UNSIGNED_SHORT, ib_);
  else if (pt == VB_Lines)
    glDrawElements(GL_LINES, ibsize_, GL_UNSIGNED_SHORT, ib_);
}

void GLESVertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.r; col->g = c.g; col->b = c.b; col->a = c.a;
}

void GLESVertexBuffer::setTexCoord(int pos, ::Math::Vec2f t, bool dxswap){
  ::Math::Vec2f* tex;
  tex = (::Math::Vec2f*)(((char*)verts_)+pos*structsize_+texoffset_);
  tex->x = t.x; tex->y = t.y;
}

void GLESVertexBuffer::setVertexOffset(int offset){
  userVertOffset_ = 4*offset*structsize_;//offset*structsize_;
  glVertexPointer(3, GL_FLOAT, structsize_, vb_+vertoffset_+userVertOffset_);
  glColorPointer(4, GL_UNSIGNED_BYTE, structsize_, vb_+coloffset_+userVertOffset_);
  glTexCoordPointer(2, GL_FLOAT, structsize_, vb_+texoffset_+userVertOffset_);
  glNormalPointer(GL_FLOAT, structsize_, vb_+normoffset_+userVertOffset_);
}
