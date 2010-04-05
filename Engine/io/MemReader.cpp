
#include "MemReader.h"
#include <cstring>

using std::min;
using std::max;
using namespace CGE;

MemReader::MemReader() : mStart(NULL), mEnd(NULL), mCurrent(NULL){

}

MemReader::MemReader(void* address, unsigned int length){
  mStart = (char*)address;
  mEnd = mStart + length;
  mCurrent = mStart;
}

MemReader::~MemReader(){
  
}

unsigned int MemReader::readBytes(unsigned char* array, unsigned int length){
  unsigned realLength = min(length,(unsigned int)(mEnd-mCurrent));
  memcpy(array, mCurrent, realLength);
  mCurrent += realLength;
  return realLength;
}

char MemReader::readChar(){
  char ret;
  unsigned realLength = min(sizeof(char),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

unsigned char MemReader::readUChar(){
  unsigned char ret;
  unsigned realLength = min(sizeof(unsigned char),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

short MemReader::readShort(){
  short ret;
  unsigned realLength = min(sizeof(short),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

unsigned short MemReader::readUShort(){
  unsigned short ret;
  unsigned realLength = min(sizeof(unsigned short),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

int MemReader::readInt(){
  int ret;
  unsigned realLength = min(sizeof(int),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

unsigned int MemReader::readUInt(){
  unsigned int ret;
  unsigned realLength = min(sizeof(unsigned int),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

int64 MemReader::readLong(){
  int64 ret;
  unsigned realLength = min(sizeof(int64),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

uint64 MemReader::readULong(){
  uint64 ret;
  unsigned realLength = min(sizeof(uint64),(unsigned)(mEnd-mCurrent));
  memcpy(&ret, mCurrent, realLength);
  mCurrent += realLength;
  return ret;
}

bool MemReader::isWorking(){
  return mCurrent >= mStart && mCurrent < mEnd && mStart < mEnd;
}

void MemReader::skip(int numberOfElements){
  unsigned realLength;
  if (numberOfElements < 0)
    realLength = max(numberOfElements, (int)(mStart-mCurrent));
  else
    realLength = min(numberOfElements, (int)(mEnd-mCurrent));
  mCurrent += realLength;
}

std::string MemReader::readString(){
  return "";
}

std::string MemReader::readLine(){
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
