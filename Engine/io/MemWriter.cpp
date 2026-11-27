
#include "MemWriter.h"
#include <cstring>
#include <algorithm>

using std::min;
using std::max;
using namespace CGE;

MemWriter::MemWriter(uint32 size) : mStart(NULL), mEnd(NULL), mCurrent(NULL){
  mStorage.reserve(size);
}

MemWriter::MemWriter(void* address, unsigned int length){
  mStorage.resize(length);
  if (length > 0)
    memcpy(&mStorage[0], address, length);
}

MemWriter::~MemWriter(){
  
}

unsigned int MemWriter::writeBytes(unsigned char* array, unsigned int length){
  size_t end = mStorage.size();
  mStorage.resize(end + length);
  if (length > 0)
    memcpy(&mStorage[end], array, length);
  return length;
}

bool MemWriter::writeChar(char c){
  mStorage.push_back((uint8)c);
  return true;
}

unsigned char MemWriter::readUChar(){
  unsigned char ret;
  unsigned realLength = min((unsigned)sizeof(unsigned char),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

short MemWriter::readShort(){
  short ret;
  unsigned realLength = min((unsigned)sizeof(short),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

unsigned short MemWriter::readUShort(){
  unsigned short ret;
  unsigned realLength = min((unsigned)sizeof(unsigned short),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

int MemWriter::readInt(){
  int ret;
  unsigned realLength = min((unsigned)sizeof(int),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

unsigned int MemWriter::readUInt(){
  unsigned int ret;
  unsigned realLength = min((unsigned)sizeof(unsigned int),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

int64 MemWriter::readLong(){
  int64 ret;
  unsigned realLength = min((unsigned)sizeof(int64),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

uint64 MemWriter::readULong(){
  uint64 ret;
  unsigned realLength = min((unsigned)sizeof(uint64),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

bool MemWriter::isWorking(){
  return mCurrent >= mStart && mCurrent < mEnd && mStart < mEnd;
}

void MemWriter::skip(int numberOfElements){
  unsigned realLength;
  if (numberOfElements < 0)
    realLength = max(numberOfElements, (int)(mStart-mCurrent));
  else
    realLength = min(numberOfElements, (int)(mEnd-mCurrent));
  mCurrent += realLength;
}

void MemWriter::gotoEnd(){
  mCurrent = mEnd;
}

void MemWriter::jumpTo(unsigned offset){
  mCurrent = mStart+offset;
  if (mCurrent > mEnd)
    mCurrent = mEnd;
}

std::string MemWriter::readString(){
  return "";
}

std::string MemWriter::readLine(){
  std::string ret;
  while(mCurrent < mEnd){
    char chr = *mCurrent++;
    if (chr == '\n')
      break;
    else if (chr == '\r')
      continue;
    ret.append(1, chr);
  }
  return ret;
}
