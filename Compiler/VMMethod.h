#ifndef VMMETHOD_H
#define VMMETHOD_H

#include <iostream>

namespace Java{
struct Code_attribute;
}

class VMMethod{
public:
	VMMethod(Java::Code_attribute* code) {mCode = code;}
	void print(std::ostream& strm);
protected:
	Java::Code_attribute* mCode;
};


#endif
