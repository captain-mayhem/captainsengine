#ifndef MEMWRITER_H
#define MEMWRITER_H

#include "Writer.h"

#include <string>
#include <fstream>
#include <vector>

namespace CGE{

class MemWriter : public Writer{
public:
  MemWriter(uint32 size=0);
  MemWriter(void* address, unsigned int length);
  virtual ~MemWriter();
  virtual uint32 writeBytes(unsigned char* array, unsigned int length);
  virtual bool writeChar(char c);
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
  virtual void gotoEnd();
  virtual void jumpTo(unsigned offset);

  virtual uint32 getSize() const { return (uint32)mStorage.size(); }

  char const* getData() const {return mStorage.empty() ? NULL : (char const*)&mStorage[0];}
  char* getCurrentData() {return mCurrent;}
protected:
  char* mStart;
  char* mEnd;
  char* mCurrent;
  std::vector<uint8> mStorage;
};

}

#endif