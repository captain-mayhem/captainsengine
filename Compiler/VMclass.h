#ifndef VMCLASS_H
#define VMCLASS_H

#include <string>
#include "JavaDefs.h"

class VMClass{
public:
	VMClass(const std::string& filename);
	void print();
protected:
	Java::ClassFile mClass;
  std::string mFilename;
};

#endif
