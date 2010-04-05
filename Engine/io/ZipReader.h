#ifndef CGE_ZIPREADER_H
#define CGE_ZIPREADER_H

#include <string>
#include <../extern/zlib/unzip.h>
#include "MemReader.h"

namespace CGE{

class ZipReader : public MemReader{
public:
  ZipReader(const std::string& zippath);
  ZipReader(void* address, uint32 length);
  MemReader openEntry(const std::string& entry);
  virtual ~ZipReader();
protected:
  unzFile mHandle;
  zlib_filefunc_def mFileFuncs;
  char* mBuffer;
  unsigned mBufferSize;
  static voidpf open_file_func(voidpf opaque, const char* filename, int mode);
  static uLong read_file_func(voidpf opaque, voidpf stream, void* buf, uLong size);
  static uLong write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size);
  static long tell_file_func(voidpf opaque, voidpf stream);
  static long seek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin);
  static int close_file_func(voidpf opaque, voidpf stream);
  static int testerror_file_func(voidpf opaque, voidpf stream);
};

}

#endif