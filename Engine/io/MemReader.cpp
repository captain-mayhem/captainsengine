
#include "MemReader.h"
#include <cstring>

using std::min;
using std::max;

MemReader::MemReader(void* address, unsigned int length){
  mStart = (char*)address;
  mEnd = mStart + length;
  mCurrent = mStart;
}

MemReader::~MemReader(){
  
}

void MemReader::readBytes(unsigned char* array, unsigned int length){
  unsigned realLength = min(length,(unsigned int)(mEnd-mCurrent));
  memcpy(array, mCurrent, realLength);
  mCurrent += realLength;
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
