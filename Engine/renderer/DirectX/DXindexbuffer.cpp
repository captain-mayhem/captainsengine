
#include "DXindexbuffer.h"
#include "DXrenderer.h"
#include "../../system/engine.h"

using namespace CGE;

DXIndexBuffer::DXIndexBuffer(Type t, int32 size) : IndexBuffer(t,size){
  mIbSize = size;
  LPDIRECT3DDEVICE9 device = dynamic_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  device->CreateIndexBuffer(size*sizeof(short),
    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib_, 0);
}

DXIndexBuffer::~DXIndexBuffer(){
  SAFE_RELEASE(ib_);
}

void* DXIndexBuffer::lockIndexPointer(){
  void* inds;
  ib_->Lock(0,0, &inds, 0);
  return inds;
}

void DXIndexBuffer::unlockIndexPointer(){
  ib_->Unlock();
}

void DXIndexBuffer::setIndex(uint32 i, uint8 index){

}

void DXIndexBuffer::setIndex(uint32 i, uint16 index){

}

void DXIndexBuffer::setIndex(uint32 i, uint32 index){

}
