#include "../../system/engine.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "GL2Shader.h"
#include <GL/gl.h>
#include "GL2vertexbuffer.h"
#include "GL2indexbuffer.h"
#include "../renderer.h"

using namespace CGE;

GL2VertexBuffer::GL2VertexBuffer(){
  vb_ = NULL;
  flags_ = 0;
  verts_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
  texoffset_ = -1;
  normoffset_ = -1;
  userVertOffset_ = 0;
}

GL2VertexBuffer::~GL2VertexBuffer(){
  SAFE_DELETE_ARRAY(vb_);
}

void GL2VertexBuffer::create(int type, int vertexBufferSize){
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

void* GL2VertexBuffer::lockVertexPointer(){
  verts_ = (void*)vb_;
  return verts_;
}

void GL2VertexBuffer::unlockVertexPointer(){
  verts_ = NULL;
}

void GL2VertexBuffer::activate(){
  if (flags_ & VB_POSITION){
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, structsize_, vb_ + vertoffset_ + userVertOffset_);
  }
  else
    glDisableVertexAttribArray(0);
  if (flags_ & VB_COLOR){
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, structsize_, vb_ + coloffset_ + userVertOffset_);
  }
  else
    glDisableVertexAttribArray(1);
  if (flags_ & VB_TEXCOORD){
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, structsize_, vb_ + texoffset_ + userVertOffset_);
  }
  else
    glDisableVertexAttribArray(2);
  if (flags_ & VB_NORMAL){
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, structsize_, vb_ + normoffset_ + userVertOffset_);
  }
  else
    glDisableVertexAttribArray(3);
}

void GL2VertexBuffer::draw(PrimitiveType pt, IndexBuffer* indices, int offset, int count){
  GL2Shader::getCurrentShader()->applyEngineUniforms();
  if (indices == NULL){
    if (pt == VB_Tristrip)
      glDrawArrays(GL_TRIANGLE_STRIP, offset, count);
    else if (pt == VB_Triangles)
      glDrawArrays(GL_TRIANGLES, offset, count);
    else if (pt == VB_Trifan)
      glDrawArrays(GL_TRIANGLE_FAN, offset, count);
    else if (pt == VB_Lines)
      glDrawArrays(GL_LINES, offset, count);
    else if (pt == VB_Points)
      glDrawArrays(GL_POINTS, offset, count);
    return;
  }
  GL2IndexBuffer* oglidx = (GL2IndexBuffer*)indices;
  if (pt == VB_Tristrip)
    glDrawElements(GL_TRIANGLE_STRIP, count, oglidx->getGLType(), oglidx->getIndices(offset));
  else if (pt == VB_Triangles)
    glDrawElements(GL_TRIANGLES, count, oglidx->getGLType(), oglidx->getIndices(offset));
  else if (pt == VB_Trifan)
    glDrawElements(GL_TRIANGLE_FAN, count, oglidx->getGLType(), oglidx->getIndices(offset));
  else if (pt == VB_Lines)
    glDrawElements(GL_LINES, count, oglidx->getGLType(), oglidx->getIndices(offset));
  else if (pt == VB_Points)
    glDrawElements(GL_POINTS, count, oglidx->getGLType(), oglidx->getIndices(offset));
}

void GL2VertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.r; col->g = c.g; col->b = c.b; col->a = c.a;
}

void GL2VertexBuffer::setTexCoord(int pos, ::CGE::Vec2f t, bool dxswap){
  ::CGE::Vec2f* tex;
  tex = (::CGE::Vec2f*)(((char*)verts_)+pos*structsize_+texoffset_);
  tex->x = t.x; tex->y = t.y;
}

void GL2VertexBuffer::setVertexOffset(int offset){
  userVertOffset_ = 4*offset*structsize_;//offset*structsize_;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, structsize_, vb_ + vertoffset_ + userVertOffset_);
  glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, structsize_, vb_ + coloffset_ + userVertOffset_);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, structsize_, vb_+texoffset_+userVertOffset_);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, structsize_, vb_ + normoffset_ + userVertOffset_);
}
