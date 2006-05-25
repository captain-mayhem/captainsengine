#ifndef DXVERTEXBUFFER_H
#define DXVERTEXBUFFER_H

#include <d3d9.h>
#include "vertexbuffer.h"

namespace Graphics{
  
class DXVertexBuffer : public VertexBuffer {
public:
  DXVertexBuffer();
  ~DXVertexBuffer();
  void create(int type, int structsize, int vertexBufferSize, int indexBufferSize);
  void* lockVertexPointer();
  short* lockIndexPointer();
  void unlockVertexPointer();
  void unlockIndexPointer();
  void draw();
protected:
  LPDIRECT3DDEVICE9 device_;
  IDirect3DVertexBuffer9* vb_;
  IDirect3DIndexBuffer9* ib_;
  DWORD flags_;
  int structsize_;
  int vbsize_;
  int ibsize_;
};

}

#endif