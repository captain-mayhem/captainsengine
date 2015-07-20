#ifndef JAVABINFILEREADER_H
#define JAVABINFILEREADER_H

#include <io/Reader.h>
#include "JavaDefs.h"

class JavaBinFileReader{
public:
  JavaBinFileReader(CGE::Reader* reader);
  virtual ~JavaBinFileReader();
  unsigned readClassFile(Java::ClassFile& jclass);
  bool isWorking() {return mReader->isWorking();}
  void setReader(CGE::Reader* reader) {mReader = reader;}
protected:
  Java::u4 readU4();
  Java::u2 readU2();
  Java::u1 readU1() {return mReader->readUChar();}
  Java::u2 readConstantPool(std::vector<Java::cp_info*>& constant_pool, Java::u2 numEntries);
  void readAttributes(std::vector<Java::attribute_info*>& attributes, Java::u2 numEntries, const Java::ClassFile& jclass);

  CGE::Reader* mReader;
};

#endif