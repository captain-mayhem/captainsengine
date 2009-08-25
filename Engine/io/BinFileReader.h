#ifndef BINFILEREADER_H
#define BINFILEREADER_H

#include "Reader.h"

#include <string>
#include <fstream>

class BinFileReader : public Reader{
public:
  BinFileReader(std::string filename);
  virtual ~BinFileReader();
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
  std::ifstream mStream;
};

#endif