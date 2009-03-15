#ifndef MEMREADER_H
#define MEMREADER_H

#include "Reader.h"

#include <string>
#include <fstream>

class MemReader : public Reader{
public:
  MemReader(void* address, unsigned int length);
  virtual ~MemReader();
  virtual void readBytes(unsigned char* array, unsigned int length);
  virtual char readChar();
  virtual unsigned char readUChar();
  virtual short readShort();
  virtual unsigned short readUShort();
  virtual int readInt();
  virtual unsigned int readUInt();

  virtual bool isWorking();
  virtual void skip(int numberOfElements);
protected:
  char* mStart;
  char* mEnd;
  char* mCurrent;
};

#endif