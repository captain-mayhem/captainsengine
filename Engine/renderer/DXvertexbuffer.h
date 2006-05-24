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
  void* getVertexPointer();
  void* getIndexPointer();
  void draw();
protected:
  LPDIRECT3DDEVICE9 device_;
  IDirect3DVertexBuffer9* vb_;
  IDirect3DIndexBuffer9* ib_;
};

}

#endif