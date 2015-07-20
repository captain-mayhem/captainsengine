#ifndef CGE_INDEXBUFFER_H
#define CGE_INDEXBUFFER_H

#include "../system/types.h"

namespace CGE{

class IndexBuffer{
public:
  enum Type{
    IB_UCHAR=1,
    IB_USHORT=2,
    IB_UINT=4,
  };
  IndexBuffer(Type t, int32 size);
  virtual ~IndexBuffer();
  virtual void* lockIndexPointer()=0;
  virtual void unlockIndexPointer()=0;
  virtual void activate() = 0;
  virtual void setIndex(uint32 i, uint8 index)=0;
  virtual void setIndex(uint32 i, uint16 index)=0;
  virtual void setIndex(uint32 i, uint32 index)=0;
  virtual uint32 getNumIndices() = 0;
protected:
  Type mType;
};

}

#endif
