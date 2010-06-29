#ifndef VMMETHOD_H
#define VMMETHOD_H

#include <iostream>

#include "jni.h"
#include "JavaDefs.h"

namespace Java{
struct Code_attribute;
}

class VMContext;
class VMClass;

class VMMethod{
public:
	virtual void print(std::ostream& strm)=0;
	virtual void execute(VMContext* ctx, VMClass* cls)=0;
};

class BcVMMethod : public VMMethod{
public:
	BcVMMethod(Java::Code_attribute* code) {mCode = code;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, VMClass* cls);
protected:
	Java::Code_attribute* mCode;
};

class NativeVMMethod : public VMMethod{
public:
	NativeVMMethod(nativeMethod mthd) {mFunction = mthd;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, VMClass* cls);
protected:
	nativeMethod mFunction;
};


#endif
