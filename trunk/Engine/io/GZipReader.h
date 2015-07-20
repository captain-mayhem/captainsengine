#ifndef CGE_GZIPREADER_H
#define CGE_GZIPREADER_H

#include <string>
#include <../extern/zlib/zlib.h>
#include "MemReader.h"

namespace CGE{

class GZipReader : public MemReader{
public:
  GZipReader();
  GZipReader(void* address, uint32 length, unsigned outputdatasize);
  virtual ~GZipReader();
protected:
  z_stream mStream;
  //char* mFileBuffer;
  //char* mBuffer;
  //unsigned mBufferSize;
};

}

#endif
