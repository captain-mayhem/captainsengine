#if defined WIN32 && !defined _NODIRECTX

#include "../../system/engine.h"
#include "DXrenderer.h"
#include "DXvertexbuffer.h"
#include "DXindexbuffer.h"

using namespace CGE;

DXVertexBuffer::DXVertexBuffer(){
  vb_ = NULL;
  device_ = dynamic_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  flags_ = 0;
  verts_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
  texoffset_ = -1;
  normoffset_ = -1;
  userVertOffset_ = 0;
}

DXVertexBuffer::~DXVertexBuffer(){
  SAFE_RELEASE(vb_);
}

void DXVertexBuffer::create(int type, int vertexBufferSize){
  vbsize_ = vertexBufferSize;
  int offset = 0;
  if (type & VB_POSITION){
    flags_ |= D3DFVF_XYZ;
    vertoffset_ = offset;
    offset += 3*sizeof(float);
  }
  if (type & VB_NORMAL){
    flags_ |= D3DFVF_NORMAL;
    normoffset_ = offset;
    offset += 3*sizeof(float);
  }
  if (type & VB_COLOR){
    flags_ |= D3DFVF_DIFFUSE;
    coloffset_ = offset;
    offset += 4*sizeof(char);
  }
  if (type & VB_TEXCOORD){
    flags_ |= D3DFVF_TEX1;
    texoffset_ = offset;
    offset += 2*sizeof(float);
  }
  structsize_ = offset;
  if (device_->CreateVertexBuffer(vertexBufferSize*structsize_, D3DUSAGE_WRITEONLY, flags_,
    D3DPOOL_MANAGED, &vb_, 0)){
      EXIT2("Cannot create vertex buffer");
    }
}

void* DXVertexBuffer::lockVertexPointer(){
  vb_->Lock(0, 0, &verts_, 0);
  return verts_;
}

void DXVertexBuffer::unlockVertexPointer(){
  vb_->Unlock();
  verts_ = NULL;
}

void DXVertexBuffer::activate(){
  device_->SetStreamSource(0, vb_, 0, structsize_);
  device_->SetFVF(flags_);
}

void DXVertexBuffer::draw(PrimitiveType pt, IndexBuffer* indices){
  //device_->SetRenderState(D3DRS_LIGHTING, false);
  if (indices == NULL){
    if (pt == VB_Tristrip)
      device_->DrawPrimitive(D3DPT_TRIANGLESTRIP, userVertOffset_, vbsize_-2);
    else if (pt == VB_Triangles)
      device_->DrawPrimitive(D3DPT_TRIANGLELIST, userVertOffset_, vbsize_/3);
    else if (pt == VB_Trifan)
      device_->DrawPrimitive(D3DPT_TRIANGLEFAN, userVertOffset_, vbsize_-2);
    else if (pt == VB_Lines)
      device_->DrawPrimitive(D3DPT_LINELIST, userVertOffset_, vbsize_/3);
    else if (pt == VB_Points)
      device_->DrawPrimitive(D3DPT_POINTLIST, userVertOffset_, vbsize_/3);
    return;
  }
  DXIndexBuffer* dxinds = (DXIndexBuffer*)indices;
  device_->SetIndices(dxinds->getIndices());
  if (pt == VB_Tristrip)
    device_->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, userVertOffset_, 0, vbsize_, 0, dxinds->getNumIndices());
  else if (pt == VB_Triangles)
    device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, userVertOffset_, 0, vbsize_, 0, dxinds->getNumIndices());
  else if (pt == VB_Trifan)
    device_->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, userVertOffset_, 0, vbsize_, 0, dxinds->getNumIndices());
  else if (pt == VB_Lines)
    device_->DrawIndexedPrimitive(D3DPT_LINELIST, userVertOffset_, 0, vbsize_, 0, dxinds->getNumIndices());
  else if (pt == VB_Points)
    device_->DrawIndexedPrimitive(D3DPT_POINTLIST, userVertOffset_, 0, vbsize_, 0, dxinds->getNumIndices());
}

void DXVertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.b; col->g = c.g; col->b = c.r; col->a = c.a;
}

void DXVertexBuffer::setTexCoord(int pos, ::CGE::Vec2f t, bool dxswap){
  ::CGE::Vec2f* tex;
  tex = (::CGE::Vec2f*)(((char*)verts_)+pos*structsize_+texoffset_);
  tex->x = t.x; 
  if (dxswap)
    tex->y = 1-t.y;
  else
    tex->y = t.y;
}

void DXVertexBuffer::setVertexOffset(int offset){
  userVertOffset_ = 4*offset;
}

#endif
