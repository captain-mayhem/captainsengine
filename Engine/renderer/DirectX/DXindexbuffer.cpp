
#include "DXindexbuffer.h"
#include "DXrenderer.h"
#include "../../system/engine.h"

using namespace CGE;

DXIndexBuffer::DXIndexBuffer(Type t, int32 size) : IndexBuffer(t,size){
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
  return inds;
}

void DXIndexBuffer::unlockIndexPointer(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->Unmap(mIb, NULL);
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

}

void DXIndexBuffer::setIndex(uint32 i, uint16 index){

}

void DXIndexBuffer::setIndex(uint32 i, uint32 index){

}
