#ifndef CGE_OGL_INDEXBUFFER_H
#define CGE_OGL_INDEXBUFFER_H

#include "../../system/types.h"
#include "../indexbuffer.h"

namespace CGE{

class OGLIndexBuffer : public IndexBuffer{
public:
  OGLIndexBuffer(Type t, int32 size);
  virtual ~OGLIndexBuffer();
  virtual void* lockIndexPointer();
  virtual void unlockIndexPointer();
  virtual void setIndex(uint32 i, uint8 index);
  virtual void setIndex(uint32 i, uint16 index);
  virtual void setIndex(uint32 i, uint32 index);
  uint32 getNumIndices() {return mIbSize;}
  void* getIndices() {return mInds;}
  uint32 getType() {return mType;}
protected:
  uint32 mType;
  uint32 mIbSize;
  uint8* mInds;
};

}

#endif
