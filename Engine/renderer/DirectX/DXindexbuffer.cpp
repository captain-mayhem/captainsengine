
#include "DXindexbuffer.h"
#include "DXrenderer.h"
#include "../../system/engine.h"

using namespace CGE;

DXIndexBuffer::DXIndexBuffer(Type t, int32 size) : IndexBuffer(t,size), mInds(NULL){
  mIbSize = size;
  ID3D11Device* device = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = size*t;
  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  device->CreateBuffer(&desc, NULL, &mIb);
}

DXIndexBuffer::~DXIndexBuffer(){
  SAFE_RELEASE(mIb);
}

void* DXIndexBuffer::lockIndexPointer(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  void* inds;
  D3D11_MAPPED_SUBRESOURCE mr;
  ctx->Map(mIb, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mr);
  inds = mr.pData;
  mInds = (unsigned char*)inds;
  return inds;
}

void DXIndexBuffer::unlockIndexPointer(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->Unmap(mIb, NULL);
  mInds = NULL;
}

void DXIndexBuffer::activate(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  DXGI_FORMAT fmt;
  if (mType == IB_UCHAR)
    fmt = DXGI_FORMAT_R8_UINT;
  else if (mType == IB_USHORT)
    fmt = DXGI_FORMAT_R16_UINT;
  else if (mType == IB_UINT)
    fmt = DXGI_FORMAT_R32_UINT;
  ctx->IASetIndexBuffer(mIb, fmt, 0);
}

void DXIndexBuffer::setIndex(uint32 i, uint8 index){
  setIndex(i, (uint32)index);
}

void DXIndexBuffer::setIndex(uint32 i, uint16 index){
  setIndex(i, (uint32)index);
}

void DXIndexBuffer::setIndex(uint32 i, uint32 index){
  switch (mType){
  case IB_UCHAR:
  {
    uint8* pos = mInds + i*sizeof(uint8);
    *pos = index;
    break;
  }
  case IB_USHORT:
  {
    uint16* pos = (uint16*)(mInds + i*sizeof(uint16));
    *pos = index;
    break;
  }
  case IB_UINT:
  {
    uint32* pos = (uint32*)(mInds + i*sizeof(uint32));
    *pos = index;
    break;
  }
  }
}
