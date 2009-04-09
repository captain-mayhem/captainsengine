#ifndef CGE_INDEXBUFFER_H
#define CGE_INDEXBUFFER_H

#include "../system/types.h"

namespace CGE{

class IndexBuffer{
public:
  enum Type{
    IB_UCHAR,
    IB_USHORT,
    IB_UINT,
  };
  IndexBuffer(Type t, int32 size);
  virtual ~IndexBuffer();
  virtual void* lockIndexPointer()=0;
  virtual void unlockIndexPointer()=0;
  virtual void setIndex(uint32 i, uint8 index)=0;
  virtual void setIndex(uint32 i, uint16 index)=0;
  virtual void setIndex(uint32 i, uint32 index)=0;
};

}

#endif
