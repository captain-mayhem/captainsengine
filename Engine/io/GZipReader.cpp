#include "GZipReader.h"

using namespace CGE;

//TR_CHANNEL(CGE_GZIPREADER)

GZipReader::GZipReader() : MemReader(NULL, 0){
}

GZipReader::GZipReader(void* address, uint32 length, unsigned outputdatasize) : MemReader(new char[outputdatasize], outputdatasize){
  mStream.zalloc = Z_NULL;
  mStream.zfree = Z_NULL;
  mStream.opaque = Z_NULL;
  mStream.avail_in = 0;
  mStream.next_in = Z_NULL;
  int ret = inflateInit(&mStream);
  if (ret != Z_OK){
  }
  mStream.avail_in = length;
  mStream.next_in = (Bytef*)address;
  
  mStream.avail_out = outputdatasize;
  mStream.next_out = (Bytef*)mStart;
  ret = inflate(&mStream, Z_NO_FLUSH);

  inflateEnd(&mStream);
}

GZipReader::~GZipReader(){
  delete [] mStart;
}

