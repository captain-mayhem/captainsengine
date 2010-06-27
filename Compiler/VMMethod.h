#ifndef VMMETHOD_H
#define VMMETHOD_H

#include <iostream>

namespace Java{
struct Code_attribute;
}

class VMContext;
class VMClass;

class VMMethod{
public:
	VMMethod(Java::Code_attribute* code) {mCode = code;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, VMClass* cls);
protected:
	Java::Code_attribute* mCode;
};


#endif
