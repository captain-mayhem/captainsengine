#ifndef DXVERTEXBUFFER_H
#define DXVERTEXBUFFER_H

#include <d3d11.h>
#include "../vertexbuffer.h"

namespace CGE{
  
class DXVertexBuffer : public VertexBuffer {
public:
  DXVertexBuffer();
  ~DXVertexBuffer();
  void create(int type, int vertexBufferSize);
  void* lockVertexPointer();
  void unlockVertexPointer();
  void activate();
  void draw(PrimitiveType pt, IndexBuffer* indices, int offset, int count);
  void setColor(int pos, Color c);
  void setTexCoord(int pos, ::CGE::Vec2f t);
  void setVertexOffset(int offset);
protected:
  ID3D11Device* mDevice;
  ID3D11Buffer* mVb;
  DWORD flags_;
};

}

#endif
