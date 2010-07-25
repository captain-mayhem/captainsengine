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
		Void=0,
		Long=11,
		Reference=1,
		Int=10,
		Boolean=4,
		Char=5,
		Array=2,
		Byte=8,
		Float=6,
		Double=7,
		Short=9
	};
	VMMethod(const std::string& name, const std::string& signature, VMClass* cls, bool isStatic) : mName(name), mSignature(signature), mIsStatic(isStatic), mClass(cls) {parseSignature();}
	virtual void execute(VMContext* ctx)=0;
	virtual void print(std::ostream& strm)=0;
	unsigned getNumArgs() {return mArgSize;}
	const std::string& getName() const {return mName;}
	const std::string& getSignature() const {return mSignature;}
	VMClass* getClass() {return mClass;}
	static ReturnType parseType(const char type);
	//virtual void executeVoidRet(VMContext* ctx, VMClass* cls)=0;
	//virtual void executeLongRet(VMContext* ctx, VMClass* cls)=0;
	//virtual void executeRefRet(VMContext* ctx, VMClass* cls)=0;
protected:
	void parseSignature();

	std::string mName;
	std::string mSignature;
	ReturnType mReturnType;
	unsigned mArgSize;
	bool mIsStatic;
	VMClass* mClass;
};

class BcVMMethod : public VMMethod{
public:
	BcVMMethod(const std::string& name, const std::string& signature, VMClass* cls, bool isStatic, Java::Code_attribute* code) : VMMethod(name, signature, cls, isStatic) {mCode = code;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx);
	void executeVoidRet(VMContext* ctx);
	void executeLongRet(VMContext* ctx);
	void executeRefRet(VMContext* ctx);
protected:
	Java::Code_attribute* mCode;
};

class NativeVMMethod : public VMMethod{
public:
	NativeVMMethod(const std::string& name, const std::string& signature, VMClass* cls, bool isStatic, nativeMethod mthd) : VMMethod(name, signature, cls, isStatic) {mFunction = mthd;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx);
	void executeVoidRet(VMContext* ctx);
	void executeLongRet(VMContext* ctx);
	void executeRefRet(VMContext* ctx);
	void executeBoolRet(VMContext* ctx);
protected:
	uint8* packArguments(VMContext* ctx);
	nativeMethod mFunction;
};


#endif
