#ifdef WIN32

#include "../system/engine.h"
#include "DXrenderer.h"
#include "DXvertexbuffer.h"

using namespace Graphics;

DXVertexBuffer::DXVertexBuffer(){
  vb_ = NULL;
  ib_ = NULL;
  device_ = dynamic_cast< ::Graphics::DXRenderer* >(::System::Engine::instance()->getRenderer())->getDevice();
  flags_ = 0;
  verts_ = NULL;
  inds_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
  texoffset_ = -1;
  userVertOffset_ = 0;
}

DXVertexBuffer::~DXVertexBuffer(){
  SAFE_RELEASE(vb_);
  SAFE_RELEASE(ib_);
}

#include "../system/engine.h"

void DXVertexBuffer::create(int type, int vertexBufferSize, int indexBufferSize){
  vbsize_ = vertexBufferSize;
  ibsize_ = indexBufferSize;
  int offset = 0;
  if (type & VB_POSITION){
    flags_ |= D3DFVF_XYZ;
    vertoffset_ = offset;
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
  if (indexBufferSize == 0)
    return;
  //::System::Log << structsize << " " << sizeof(short) << "\n";
  device_->CreateIndexBuffer(indexBufferSize*sizeof(short),
    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib_, 0);
}

void* DXVertexBuffer::lockVertexPointer(){
  vb_->Lock(0, 0, &verts_, 0);
  return verts_;
}

short* DXVertexBuffer::lockIndexPointer(){
  ib_->Lock(0,0, (void**)&inds_, 0);
  return inds_;
}

void DXVertexBuffer::unlockVertexPointer(){
  vb_->Unlock();
  verts_ = NULL;
}

void DXVertexBuffer::unlockIndexPointer(){
  ib_->Unlock();
  inds_ = NULL;
}

void DXVertexBuffer::activate(){
  device_->SetStreamSource(0, vb_, 0, structsize_);
  device_->SetIndices(ib_);
  device_->SetFVF(flags_);
}

void DXVertexBuffer::draw(PrimitiveType pt){
  //device_->SetRenderState(D3DRS_LIGHTING, false);
  if (pt == VB_Tristrip)
    device_->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, userVertOffset_, 0, vbsize_, 0, ibsize_);
  else if (pt == VB_Triangles)
    device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, userVertOffset_, 0, vbsize_, 0, ibsize_);
}

void DXVertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.b; col->g = c.g; col->b = c.r; col->a = c.a;
}

void DXVertexBuffer::setTexCoord(int pos, ::Math::Vec2f t){
  ::Math::Vec2f* tex;
  tex = (::Math::Vec2f*)(((char*)verts_)+pos*structsize_+texoffset_);
  tex->x = t.x; tex->y = 1-t.y;
}

void DXVertexBuffer::setVertexOffset(int offset){
  userVertOffset_ = 4*offset;
}

#endif
