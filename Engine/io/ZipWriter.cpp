#include "ZipWriter.h"

#include <cstring>
#include <ctime>

using namespace CGE;

ZipWriter::ZipWriter() : mHandle(NULL), mFailed(false), mPos(0){
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

ZipWriter::ZipWriter(const std::string& zippath) : mHandle(NULL), mFailed(false), mPos(0){
  createFile(zippath);
}

ZipWriter::~ZipWriter(){
  close();
}

bool ZipWriter::createFile(const std::string& filename){
  close();
  mFailed = false;
  mHandle = zipOpen(filename.c_str(), APPEND_STATUS_CREATE);
  return mHandle != NULL;
}

bool ZipWriter::close(){
  if (!mHandle)
    return !mFailed;
  if (zipClose(mHandle, NULL) != ZIP_OK)
    mFailed = true;
  mHandle = NULL;
  return !mFailed;
}

namespace{

//! the entry's time: now, as the C# StoryDesigner's zip library writes it
zip_fileinfo fileInfo(){
  zip_fileinfo info;
  memset(&info, 0, sizeof(info));
  time_t now = time(NULL);
  struct tm* t = localtime(&now);
  if (t){
    info.tmz_date.tm_sec = t->tm_sec;
    info.tmz_date.tm_min = t->tm_min;
    info.tmz_date.tm_hour = t->tm_hour;
    info.tmz_date.tm_mday = t->tm_mday;
    info.tmz_date.tm_mon = t->tm_mon;
    info.tmz_date.tm_year = t->tm_year + 1900;
  }
  return info;
}

}

bool ZipWriter::addEntry(const std::string& entry, Reader& file, const std::string& password){
  if (!mHandle)
    return false;
  // the encryption header needs the entry's CRC, so the data is read first
  std::vector<uint8> data;
  uint32 len;
  do {
    uint8 tmp[4096];
    len = file.readBytes(tmp, 4096);
    data.insert(data.end(), tmp, tmp+len);
  } while (len > 0);
  return addEntry(entry, data.empty() ? NULL : &data[0], data.size(), password);
}

bool ZipWriter::addEntry(const std::string& entry, const void* data, size_t size, const std::string& password){
  if (!mHandle)
    return false;
  zip_fileinfo info = fileInfo();
  int err;
  if (password.empty()){
    err = zipOpenNewFileInZip(mHandle, entry.c_str(), &info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
  }
  else{
    uLong crc = crc32(0L, Z_NULL, 0);
    if (size > 0)
      crc = crc32(crc, (const Bytef*)data, (uInt)size);
    err = zipOpenNewFileInZip3(mHandle, entry.c_str(), &info, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION,
      0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, password.c_str(), crc);
  }
  if (err != ZIP_OK){
    mFailed = true;
    return false;
  }
  if (size > 0 && zipWriteInFileInZip(mHandle, data, (unsigned)size) != ZIP_OK)
    mFailed = true;
  if (zipCloseFileInZip(mHandle) != ZIP_OK)
    mFailed = true;
  return !mFailed;
}

voidpf ZipWriter::open_file_func(voidpf opaque, const char* filename, int mode){
  ZipWriter* wrt = static_cast<ZipWriter*>(opaque);
  wrt->mData.clear();
  wrt->mPos = 0;
  return wrt;
}

uLong ZipWriter::read_file_func(voidpf opaque, voidpf stream, void* buf, uLong size){
  ZipWriter* wrt = static_cast<ZipWriter*>(stream);
  if (wrt->mPos >= wrt->mData.size())
    return 0;
  uLong len = (uLong)(wrt->mData.size()-wrt->mPos);
  if (len > size)
    len = size;
  memcpy(buf, &wrt->mData[wrt->mPos], len);
  wrt->mPos += len;
  return len;
}

uLong ZipWriter::write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size){
  ZipWriter* wrt = static_cast<ZipWriter*>(stream);
  // minizip seeks back to patch the local headers, so this overwrites as well as appends
  if (wrt->mPos+size > wrt->mData.size())
    wrt->mData.resize(wrt->mPos+size);
  if (size > 0)
    memcpy(&wrt->mData[wrt->mPos], buf, size);
  wrt->mPos += size;
  return size;
}

long ZipWriter::tell_file_func(voidpf opaque, voidpf stream){
  ZipWriter* wrt = static_cast<ZipWriter*>(stream);
  return (long)wrt->mPos;
}

long ZipWriter::seek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin){
  ZipWriter* wrt = static_cast<ZipWriter*>(stream);
  size_t base;
  if (origin == ZLIB_FILEFUNC_SEEK_CUR)
    base = wrt->mPos;
  else if (origin == ZLIB_FILEFUNC_SEEK_END)
    base = wrt->mData.size();
  else if (origin == ZLIB_FILEFUNC_SEEK_SET)
    base = 0;
  else
    return -1;
  wrt->mPos = base+offset;
  return 0;
}

int ZipWriter::close_file_func(voidpf opaque, voidpf stream){
  return 0;
}

int ZipWriter::testerror_file_func(voidpf opaque, voidpf stream){
  return 0;
}
