#include "ZipWriter.h"

using namespace CGE;

ZipWriter::ZipWriter() : MemWriter(NULL, 0), mFileBuffer(NULL), mBuffer(NULL), mBufferSize(0), mHandle(NULL){
}

ZipWriter::ZipWriter(const std::string& zippath) : MemWriter(NULL, 0), mFileBuffer(NULL), mBuffer(NULL), mBufferSize(0), mHandle(NULL){
  createFile(zippath);
}

ZipWriter::ZipWriter(void* address, uint32 length) : MemWriter(address, length), mFileBuffer(NULL), mBuffer(NULL), mBufferSize(0), mHandle(NULL){
  mFileFuncs.opaque = this;
  mFileFuncs.zclose_file = close_file_func;
  mFileFuncs.zerror_file = testerror_file_func;
  mFileFuncs.zopen_file = open_file_func;
  mFileFuncs.zread_file = read_file_func;
  mFileFuncs.zseek_file = seek_file_func;
  mFileFuncs.ztell_file = tell_file_func;
  mFileFuncs.zwrite_file = write_file_func;
  mHandle = zipOpen2("memory", APPEND_STATUS_CREATE, NULL, &mFileFuncs);
}

ZipWriter::~ZipWriter(){
  zipClose(mHandle, NULL);
  delete [] mFileBuffer;
  delete [] mBuffer;
}

bool ZipWriter::createFile(const std::string& filename){
  mHandle = zipOpen(filename.c_str(), APPEND_STATUS_CREATE);
  return mHandle != NULL;
}

bool ZipWriter::addEntry(const std::string& entry, Reader& file, const std::string& password){
  if (!mHandle)
    return false;
  zipOpenNewFileInZip(mHandle, entry.c_str(), NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
  uint32 len;
  do {
    uint8 tmp[4096];
    len = file.readBytes(tmp, 4096);
    zipWriteInFileInZip(mHandle, tmp, len);
  } while (len > 0);
  zipCloseFileInZip(mHandle);
  return true;
}

voidpf ZipWriter::open_file_func(voidpf opaque, const char* filename, int mode){
  ZipWriter* rdr = static_cast<ZipWriter*>(opaque);
  return rdr;
}

uLong ZipWriter::read_file_func(voidpf opaque, voidpf stream, void* buf, uLong size){
  ZipWriter* rdr = static_cast<ZipWriter*>(stream);
  return 0;
}

uLong ZipWriter::write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size){
  ZipWriter* rdr = static_cast<ZipWriter*>(stream);
  return rdr->writeBytes((uint8*)buf, size);
}

long ZipWriter::tell_file_func(voidpf opaque, voidpf stream){
  ZipWriter* rdr = static_cast<ZipWriter*>(stream);
  return (long)(rdr->mCurrent-rdr->mStart);
}

long ZipWriter::seek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin){
  ZipWriter* rdr = static_cast<ZipWriter*>(stream);
  if (origin == ZLIB_FILEFUNC_SEEK_CUR)
    rdr->skip(offset);
  else if (origin == ZLIB_FILEFUNC_SEEK_END)
    rdr->mCurrent = rdr->mEnd-offset;
  else if (origin == ZLIB_FILEFUNC_SEEK_SET)
    rdr->mCurrent = rdr->mStart+offset;
  return 0;
}

int ZipWriter::close_file_func(voidpf opaque, voidpf stream){
  return 0;
}

int ZipWriter::testerror_file_func(voidpf opaque, voidpf stream){
  return 0;
}

