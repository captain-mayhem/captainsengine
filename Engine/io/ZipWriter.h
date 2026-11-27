#ifndef CGE_ZIPWRITER_H
#define CGE_ZIPWRITER_H

#include <cge_export.h>

#include <string>
#include <vector>
#include <zip.h>
#include "Reader.h"

namespace CGE{

//! writes a zip archive, into a file or into memory; entries can be
//! encrypted with a password (the classic zip encryption ZipReader reads)
class CGE_API ZipWriter{
public:
  //! an archive in memory: getData() has it after close()
  ZipWriter();
  //! an archive in the file zippath (created, or replaced)
  explicit ZipWriter(const std::string& zippath);
  //! closes the archive
  ~ZipWriter();
  ZipWriter(const ZipWriter&) = delete;
  ZipWriter& operator=(const ZipWriter&) = delete;
  //! starts an archive in the file filename (closing the one before)
  bool createFile(const std::string& filename);
  //! adds an entry with the rest of file
  bool addEntry(const std::string& entry, Reader& file, const std::string& password = "");
  //! adds an entry with size bytes from data
  bool addEntry(const std::string& entry, const void* data, size_t size, const std::string& password = "");
  //! writes the archive's directory; true when everything was written
  bool close();
  bool isWorking() const { return mHandle != nullptr; }
  //! the archive in memory, complete after close()
  const std::vector<uint8>& getData() const { return mData; }
private:
  zipFile mHandle;
  bool mFailed;
  zlib_filefunc_def mFileFuncs;
  std::vector<uint8> mData;
  size_t mPos;
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
