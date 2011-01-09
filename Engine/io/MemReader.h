#ifndef MEMREADER_H
#define MEMREADER_H

#include "Reader.h"

#include <string>
#include <fstream>

namespace CGE{

class MemReader : public Reader{
public:
  MemReader();
  MemReader(void* address, unsigned int length);
  virtual ~MemReader();
  virtual uint32 readBytes(unsigned char* array, unsigned int length);
  virtual char readChar();
  virtual unsigned char readUChar();
  virtual short readShort();
  virtual unsigned short readUShort();
  virtual int readInt();
  virtual unsigned int readUInt();
  virtual int64 readLong();
  virtual uint64 readULong();

  virtual std::string readString();
  virtual std::string readLine();

  virtual bool isWorking();
  virtual void skip(int numberOfElements);

  virtual uint32 getSize() {return mEnd-mStart;}

  char* getData() {return mStart;}
protected:
  char* mStart;
  char* mEnd;
  char* mCurrent;
};

}

#endif