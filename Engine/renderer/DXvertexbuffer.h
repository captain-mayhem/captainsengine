#ifndef DXVERTEXBUFFER_H
#define DXVERTEXBUFFER_H

#include <d3d9.h>
#include "vertexbuffer.h"

namespace Graphics{
  
class DXVertexBuffer : public VertexBuffer {
public:
  DXVertexBuffer();
  ~DXVertexBuffer();
  void create(int type, int vertexBufferSize, int indexBufferSize);
  void* lockVertexPointer();
  short* lockIndexPointer();
  void unlockVertexPointer();
  void unlockIndexPointer();
  void activate();
  void draw(PrimitiveType pt);
  void setColor(int pos, Color c);
  void setTexCoord(int pos, ::Math::Vec2f t, bool dxswap=false);
  void setVertexOffset(int offset);
protected:
  LPDIRECT3DDEVICE9 device_;
  IDirect3DVertexBuffer9* vb_;
  IDirect3DIndexBuffer9* ib_;
  DWORD flags_;
};

}

#endif