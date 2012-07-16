#include "ZipReader.h"

using namespace CGE;

ZipReader::ZipReader() : MemReader(NULL, 0), mFileBuffer(NULL), mBuffer(NULL), mBufferSize(0), mHandle(NULL){
}

ZipReader::ZipReader(const std::string& zippath) : MemReader(NULL, 0), mFileBuffer(NULL), mBuffer(NULL), mBufferSize(0), mHandle(NULL){
  openFile(zippath);
}

ZipReader::ZipReader(void* address, uint32 length) : MemReader(address, length), mFileBuffer(NULL), mBuffer(NULL), mBufferSize(0), mHandle(NULL){
#ifdef WIN32
  mFile = INVALID_HANDLE_VALUE;
#endif
  mFileFuncs.opaque = this;
  mFileFuncs.zclose_file = close_file_func;
  mFileFuncs.zerror_file = testerror_file_func;
  mFileFuncs.zopen_file = open_file_func;
  mFileFuncs.zread_file = read_file_func;
  mFileFuncs.zseek_file = seek_file_func;
  mFileFuncs.ztell_file = tell_file_func;
  mFileFuncs.zwrite_file = write_file_func;
  mHandle = unzOpen2("memory", &mFileFuncs);
}

ZipReader::~ZipReader(){
  unzClose(mHandle);
  delete [] mFileBuffer;
  delete [] mBuffer;
#ifdef WIN32
  if (mFile != INVALID_HANDLE_VALUE){
    UnmapViewOfFile(mStart);
    mStart = NULL;
    CloseHandle(mFmap);
    CloseHandle(mFile);
  }
#endif
}

bool ZipReader::openFile(const std::string& filename){
	mFileFuncs.opaque = this;
  mFileFuncs.zclose_file = close_file_func;
  mFileFuncs.zerror_file = testerror_file_func;
  mFileFuncs.zopen_file = open_file_func;
  mFileFuncs.zread_file = read_file_func;
  mFileFuncs.zseek_file = seek_file_func;
  mFileFuncs.ztell_file = tell_file_func;
  mFileFuncs.zwrite_file = write_file_func;
#ifdef WIN32
  mFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (mFile == INVALID_HANDLE_VALUE)
    return false;
  mFmap = CreateFileMapping(mFile, NULL, PAGE_READONLY, 0, 0, NULL);
  void* data = MapViewOfFile(mFmap, FILE_MAP_READ, 0, 0, 0);
  MEMORY_BASIC_INFORMATION meminfo;
  uint32 size = VirtualQuery(data, &meminfo, sizeof(meminfo));
  mStart = (char*)data;
  mCurrent = mStart;
  mEnd = mStart+meminfo.RegionSize;
#else
  FILE* f = fopen(filename.c_str(), "rb");
  if (!f)
    return false;
  unsigned bufferSize = -ftell(f);
  fseek(f, 0, SEEK_END);
  bufferSize += ftell(f);
  fseek(f, 0, SEEK_SET);
  static char* buffer = NULL;
  mFileBuffer = new char[bufferSize];
  fread(mFileBuffer, 1, bufferSize, f);
  mStart = mFileBuffer;
  mEnd = mStart+bufferSize;
  mCurrent = mStart;
  fclose(f);
#endif
  mHandle = unzOpen2("memory", &mFileFuncs);
  //a try to speed up things
  char tmpname[1024];
  int ret = unzGoToFirstFile(mHandle);
  do {
    unzGetCurrentFileInfo(mHandle, NULL, tmpname, 1024, NULL, 0, NULL, 0);
    unz_file_pos pos;
    unzGetFilePos(mHandle, &pos);
    mOffsets[tmpname] = pos;
    ret = unzGoToNextFile(mHandle);
  } while (ret == UNZ_OK);
  return true;
}

MemReader ZipReader::openEntry(const std::string& entry, const std::string& password){
  if (!mHandle)
    return MemReader(NULL, 0);
  std::map<std::string, unz_file_pos>::iterator iter = mOffsets.find(entry);
  if (iter == mOffsets.end()){
    int ret = unzLocateFile(mHandle, entry.c_str(), 2);
    unz_file_pos pos;
    unzGetFilePos(mHandle, &pos);
    mOffsets[entry] = pos;
    if (ret != 0){
      return MemReader(NULL, 0);
    }
  }
  else{
    unzGoToFilePos(mHandle, &iter->second);
  }
  if (password.empty())
    unzOpenCurrentFile(mHandle);
  else
    unzOpenCurrentFilePassword(mHandle, password.c_str());
  unz_file_info info;
  unzGetCurrentFileInfo(mHandle, &info, NULL, 0, NULL, 0, NULL, 0);
  if (info.uncompressed_size > mBufferSize){
    delete [] mBuffer;
    mBufferSize = info.uncompressed_size;
    mBuffer = new char[info.uncompressed_size];
  }
  MemReader rdr(mBuffer, info.uncompressed_size);
  unzReadCurrentFile(mHandle, mBuffer, info.uncompressed_size);
  unzCloseCurrentFile(mHandle);
  return rdr;
}

voidpf ZipReader::open_file_func(voidpf opaque, const char* filename, int mode){
  ZipReader* rdr = static_cast<ZipReader*>(opaque);
  return rdr;
}

uLong ZipReader::read_file_func(voidpf opaque, voidpf stream, void* buf, uLong size){
  ZipReader* rdr = static_cast<ZipReader*>(stream);
  return rdr->readBytes((uint8*)buf, size);
}

uLong ZipReader::write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size){
  ZipReader* rdr = static_cast<ZipReader*>(stream);
  return 0;
}

long ZipReader::tell_file_func(voidpf opaque, voidpf stream){
  ZipReader* rdr = static_cast<ZipReader*>(stream);
  return rdr->mCurrent-rdr->mStart;
}

long ZipReader::seek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin){
  ZipReader* rdr = static_cast<ZipReader*>(stream);
  if (origin == ZLIB_FILEFUNC_SEEK_CUR)
    rdr->skip(offset);
  else if (origin == ZLIB_FILEFUNC_SEEK_END)
    rdr->mCurrent = rdr->mEnd-offset;
  else if (origin == ZLIB_FILEFUNC_SEEK_SET)
    rdr->mCurrent = rdr->mStart+offset;
  return 0;
}

int ZipReader::close_file_func(voidpf opaque, voidpf stream){
  return 0;
}

int ZipReader::testerror_file_func(voidpf opaque, voidpf stream){
  return 0;
}

uint64 ZipReader::getPos(){
  unz_file_pos pos;
  unzGetFilePos(mHandle, &pos);
  uint64 ret = pos.pos_in_zip_directory;
  ret |= ((uint64)pos.num_of_file << 32);
  return ret;
}

void ZipReader::setPos(uint64 pos){
  unz_file_pos seekpos;
  seekpos.pos_in_zip_directory = (uint32)(pos & 0xffffffff);
  seekpos.num_of_file = (uint32)((pos >> 32) & 0xffffffff);
  unzGoToFilePos(mHandle, &seekpos);
}
