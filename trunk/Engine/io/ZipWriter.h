#ifndef CGE_ZIPWRITER_H
#define CGE_ZIPWRITER_H

#include <string>
#include <map>
#include <../extern/zlib/zip.h>
#include "MemWriter.h"
#include "Reader.h"

namespace CGE{

class ZipWriter : public MemWriter{
public:
  ZipWriter();
  ZipWriter(const std::string& zippath);
  ZipWriter(void* address, uint32 length);
	bool createFile(const std::string& filename);
  bool addEntry(const std::string& entry, Reader& file, const std::string& password = "");
  virtual ~ZipWriter();
  virtual bool isWorking() { return mHandle != NULL; }
protected:
  zipFile mHandle;
  zlib_filefunc_def mFileFuncs;
  char* mFileBuffer;
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
