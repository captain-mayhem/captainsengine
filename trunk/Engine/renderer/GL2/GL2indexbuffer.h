#ifndef CGE_OGL_INDEXBUFFER_H
#define CGE_OGL_INDEXBUFFER_H

#include "../../system/types.h"
#include "../indexbuffer.h"

namespace CGE{

class GL2IndexBuffer : public IndexBuffer{
public:
  GL2IndexBuffer(Type t, int32 size);
  virtual ~GL2IndexBuffer();
  virtual void* lockIndexPointer();
  virtual void unlockIndexPointer();
  virtual void activate(){}
  virtual void setIndex(uint32 i, uint8 index);
  virtual void setIndex(uint32 i, uint16 index);
  virtual void setIndex(uint32 i, uint32 index);
  uint32 getNumIndices() {return mIbSize;}
  void* getIndices() { return mInds; }
  void* getIndices(int offset) {return mInds+offset*mType;}
  uint32 getGLType() {return mGLType;}
protected:
  uint32 mGLType;
  uint32 mIbSize;
  uint8* mInds;
};

}

#endif

