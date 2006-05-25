#include "../system/engine.h"
#include "DXrenderer.h"
#include "DXvertexbuffer.h"

using namespace Graphics;

DXVertexBuffer::DXVertexBuffer(){
  vb_ = NULL;
  ib_ = NULL;
  device_ = dynamic_cast< ::Graphics::DXRenderer* >(::System::Engine::instance()->getRenderer())->getDevice();
  flags_ = 0;
}

DXVertexBuffer::~DXVertexBuffer(){
  SAFE_RELEASE(vb_);
  SAFE_RELEASE(ib_);
}

void DXVertexBuffer::create(int type, int structsize, int vertexBufferSize, int indexBufferSize){
  structsize_ = structsize;
  vbsize_ = vertexBufferSize;
  ibsize_ = indexBufferSize;
  if (type & VB_POSITION)
    flags_ |= D3DFVF_XYZ;
  device_->CreateVertexBuffer(vertexBufferSize*structsize, D3DUSAGE_WRITEONLY, flags_,
      D3DPOOL_MANAGED, &vb_, 0);
  if (indexBufferSize == 0)
    return;
  device_->CreateIndexBuffer(indexBufferSize*sizeof(WORD),
    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib_, 0);
}

void* DXVertexBuffer::lockVertexPointer(){
  void* vertices;
  vb_->Lock(0, 0, &vertices, 0);
  return vertices;
}

short* DXVertexBuffer::lockIndexPointer(){
  short* indices;
  ib_->Lock(0,0, (void**)&indices, 0);
  return indices;
}

void DXVertexBuffer::unlockVertexPointer(){
  vb_->Unlock();
}

void DXVertexBuffer::unlockIndexPointer(){
  ib_->Unlock();
}

void DXVertexBuffer::draw(){
  device_->SetStreamSource(0, vb_, 0, structsize_);
  device_->SetIndices(ib_);
  device_->SetFVF(flags_);
  device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vbsize_, 0, ibsize_);
}