#include "../system/engine.h"
#include "DXrenderer.h"
#include "DXvertexbuffer.h"

using namespace Graphics;

DXVertexBuffer::DXVertexBuffer(){
  vb_ = NULL;
  ib_ = NULL;
  device_ = dynamic_cast< ::Graphics::DXRenderer* >(::System::Engine::instance()->getRenderer())->getDevice();
}

DXVertexBuffer::~DXVertexBuffer(){

}

void DXVertexBuffer::create(int type, int vertexBufferSize, int indexBufferSize){
  DWORD flags = 0;
  if (type & VB_POSITION)
    flags |= D3DFVF_XYZ;
  device_->CreateVertexBuffer(vertexBufferSize, D3DUSAGE_WRITEONLY, flags,
      D3DPOOL_MANAGED, &vb_, 0);
  if (indexBufferSize == 0)
    return;
  device_->CreateIndexBuffer(indexBufferSize*sizeof(WORD),
    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib_, 0);
}

void* DXVertexBuffer::getVertexPointer(){
  return NULL;
}

void* DXVertexBuffer::getIndexPointer(){
  return NULL;
}

void DXVertexBuffer::draw(){

}