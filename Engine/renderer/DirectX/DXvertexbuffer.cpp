#if defined WIN32 && !defined _NODIRECTX

#include "../../system/engine.h"
#include "DXrenderer.h"
#include "DXvertexbuffer.h"
#include "DXindexbuffer.h"
#include "../shader.h"

using namespace CGE;

DXVertexBuffer::DXVertexBuffer(){
  mVb = NULL;
  mDevice = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  flags_ = 0;
  verts_ = NULL;
  vertoffset_ = -1;
  coloffset_ = -1;
  texoffset_ = -1;
  tex2offset_ = -1;
  normoffset_ = -1;
  userVertOffset_ = 0;
}

DXVertexBuffer::~DXVertexBuffer(){
  SAFE_RELEASE(mVb);
}

void DXVertexBuffer::create(int type, int vertexBufferSize){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  vbsize_ = vertexBufferSize;
  int offset = 0;
  if (type & VB_POSITION){
    vertoffset_ = offset;
    offset += 3*sizeof(float);
  }
  if (type & VB_COLOR){
    coloffset_ = offset;
    offset += 4 * sizeof(char);
  }
  if (type & VB_TEXCOORD){
    texoffset_ = offset;
    offset += 2*sizeof(float);
  }
  if (type & VB_NORMAL){
    normoffset_ = offset;
    offset += 3 * sizeof(float);
  }
  if (type & VB_TEXCOORD2){
    tex2offset_ = offset;
    offset += 2 * sizeof(float);
  }
  structsize_ = offset;

  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = structsize_*vertexBufferSize;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  mDevice->CreateBuffer(&desc, NULL, &mVb);
}

void* DXVertexBuffer::lockVertexPointer(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  D3D11_MAPPED_SUBRESOURCE mr;
  ctx->Map(mVb, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mr);
  verts_ = mr.pData;
  return verts_;
}

void DXVertexBuffer::unlockVertexPointer(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->Unmap(mVb, NULL);
  verts_ = NULL;
}

void DXVertexBuffer::activate(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  UINT stride = structsize_;
  UINT offset = 0;
  ctx->IASetVertexBuffers(0, 1, &mVb, &stride, &offset);
}

void DXVertexBuffer::draw(PrimitiveType pt, IndexBuffer* indices, int offset, int count){
  Shader::getCurrentShader()->applyEngineUniforms();
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();

  if (pt == VB_Tristrip)
    ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  else if (pt == VB_Triangles)
    ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  //else if (pt == VB_Trifan)
  //ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN);
  else if (pt == VB_Lines)
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  else if (pt == VB_Linestrip)
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
  else if (pt == VB_Points)
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

  if (indices == NULL){
    ctx->Draw(count, userVertOffset_+offset);
    return;
  }
  DXIndexBuffer* dxinds = (DXIndexBuffer*)indices;
  ctx->DrawIndexed(count, offset, userVertOffset_);
}

void DXVertexBuffer::setColor(int pos, Color c){
  Color* col;
  col = (Color*)(((char*)verts_)+pos*structsize_+coloffset_);
  col->r = c.b; col->g = c.g; col->b = c.r; col->a = c.a;
}

void DXVertexBuffer::setTexCoord(int pos, ::CGE::Vec2f t){
  ::CGE::Vec2f* tex;
  tex = (::CGE::Vec2f*)(((char*)verts_)+pos*structsize_+texoffset_);
  tex->x = t.x; 
  tex->y = t.y;
}

void DXVertexBuffer::setVertexOffset(int offset){
  userVertOffset_ = 4*offset;
}

#endif
