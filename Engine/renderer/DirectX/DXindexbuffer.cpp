
#include "DXindexbuffer.h"

using namespace CGE;

DXIndexBuffer::DXIndexBuffer(Type t, int32 size) : IndexBuffer(t,size){
  LPDIRECT3DDEVICE9 device = dynamic_cast< ::Graphics::DXRenderer* >(::System::Engine::instance()->getRenderer())->getDevice();
  device->CreateIndexBuffer(size*sizeof(short),
    D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib_, 0);
}

DXIndexBuffer::~DXIndexBuffer(){
  SAFE_RELEASE(ib_);
}

void* DXIndexBuffer::lockIndexPointer(){
  void* inds;
  ib_->Lock(0,0, &inds_, 0);
  return inds;
}

void DXIndexBuffer::unlockIndexPointer(){
  ib_->Unlock();
}
