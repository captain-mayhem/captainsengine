#ifndef READER_H
#define READER_H

#include <system/types.h>
#include <string>

namespace CGE{

class Reader{
public:
  virtual uint32 readBytes(uint8* array, uint32 length) = 0;
  virtual int8 readChar() = 0;
  virtual uint8 readUChar() = 0;
  virtual int16 readShort() = 0;
  virtual uint16 readUShort() = 0;
  virtual int32 readInt() = 0;
  virtual uint32 readUInt() = 0;
  virtual int64 readLong() = 0;
  virtual uint64 readULong() = 0;

  virtual std::string readString() = 0;
  virtual std::string readLine() = 0;
  
  virtual bool isWorking() = 0;
  virtual void skip(int numberOfElements) = 0;
  virtual void gotoEnd() = 0;
  virtual void jumpTo(unsigned offset) = 0;

  virtual uint32 getSize() = 0;
};

}


#endif