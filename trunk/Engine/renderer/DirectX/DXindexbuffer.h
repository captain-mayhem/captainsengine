#ifndef CGE_DX_INDEXBUFFER_H
#define CGE_DX_INDEXBUFFER_H

#include <d3d11.h>
#include "../../system/types.h"
#include "../indexbuffer.h"

namespace CGE{

class DXIndexBuffer : public IndexBuffer{
public:
  DXIndexBuffer(Type t, int32 size);
  virtual ~DXIndexBuffer();
  virtual void* lockIndexPointer();
  virtual void unlockIndexPointer();
  virtual void activate();
  virtual void setIndex(uint32 i, uint8 index);
  virtual void setIndex(uint32 i, uint16 index);
  virtual void setIndex(uint32 i, uint32 index);

  ID3D11Buffer* getIndices() { return mIb; }
  uint32 getNumIndices() {return mIbSize;}
  //void* getIndices() {return mInds;}
  //uint32 getType() {return mType;}
protected:
  uint32 mIbSize;
  ID3D11Buffer* mIb;
};

}

#endif