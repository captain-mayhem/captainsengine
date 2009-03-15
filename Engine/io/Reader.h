#ifndef READER_H
#define READER_H

class Reader{
public:
  virtual void readBytes(unsigned char* array, unsigned int length) = 0;
  virtual char readChar() = 0;
  virtual unsigned char readUChar() = 0;
  virtual short readShort() = 0;
  virtual unsigned short readUShort() = 0;
  virtual int readInt() = 0;
  virtual unsigned int readUInt() = 0;
  
  virtual bool isWorking() = 0;
  virtual void skip(int numberOfElements) = 0;
};


#endif