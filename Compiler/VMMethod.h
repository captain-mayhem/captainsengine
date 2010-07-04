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
	enum ReturnType{
		Void,
		Long,
		Reference,
	};
	VMMethod(const std::string& name, const std::string& signature) {mName = name; mSignature = signature; parseSignature();}
	virtual void execute(VMContext* ctx, VMClass* cls)=0;
	virtual void print(std::ostream& strm)=0;
	//virtual void executeVoidRet(VMContext* ctx, VMClass* cls)=0;
	//virtual void executeLongRet(VMContext* ctx, VMClass* cls)=0;
	//virtual void executeRefRet(VMContext* ctx, VMClass* cls)=0;
protected:
	void parseSignature();
	std::string mName;
	std::string mSignature;
	ReturnType mReturnType;
};

class BcVMMethod : public VMMethod{
public:
	BcVMMethod(const std::string& name, const std::string& signature, Java::Code_attribute* code) : VMMethod(name, signature) {mCode = code;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, VMClass* cls);
	void executeVoidRet(VMContext* ctx, VMClass* cls);
	void executeLongRet(VMContext* ctx, VMClass* cls);
	void executeRefRet(VMContext* ctx, VMClass* cls);
protected:
	Java::Code_attribute* mCode;
};

class NativeVMMethod : public VMMethod{
public:
	NativeVMMethod(const std::string& name, const std::string& signature, nativeMethod mthd) : VMMethod(name, signature) {mFunction = mthd;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, VMClass* cls);
	void executeVoidRet(VMContext* ctx, VMClass* cls);
	void executeLongRet(VMContext* ctx, VMClass* cls);
	void executeRefRet(VMContext* ctx, VMClass* cls);
protected:
	nativeMethod mFunction;
};


#endif
