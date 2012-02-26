#ifndef CGE_ZIPREADER_H
#define CGE_ZIPREADER_H

#include <string>
#include <map>
#include <../extern/zlib/unzip.h>
#include "MemReader.h"

namespace CGE{

class ZipReader : public MemReader{
public:
	ZipReader();
  ZipReader(const std::string& zippath);
  ZipReader(void* address, uint32 length);
	bool openFile(const std::string& filename);
  MemReader openEntry(const std::string& entry, const std::string& password="");
  virtual ~ZipReader();
  uint64 getPos();
  void setPos(uint64 pos);
protected:
#ifdef WIN32
  HANDLE mFile;
  HANDLE mFmap;
#endif
  unzFile mHandle;
  zlib_filefunc_def mFileFuncs;
  char* mFileBuffer;
  char* mBuffer;
  unsigned mBufferSize;
  std::map<std::string, unz_file_pos> mOffsets;
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
