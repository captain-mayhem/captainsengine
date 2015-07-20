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
	VMMethod(const std::string& name, const std::string& signature, VMClass* cls, bool isStatic, int methodIndex) : 
    mName(name), mSignature(signature), mIsStatic(isStatic), mClass(cls), mRefCount(1),
    mMethodIndex(methodIndex){parseSignature();}
  //virtual ~VMMethod();
	virtual void execute(VMContext* ctx, unsigned ret)=0;
	virtual void print(std::ostream& strm)=0;
	unsigned getNumArgs() {return mArgSize;}
  const std::string& getArgument(int argnum) {return mSplitSignature[argnum];}
	const std::string& getName() const {return mName;}
	const std::string& getSignature() const {return mSignature;}
	VMClass* getClass() {return mClass;}
	static ReturnType parseType(const char type);
  void ref() {++mRefCount;}
  void unref() {--mRefCount; if (mRefCount <= 0) delete this;}
  bool handleException(unsigned* ip, VMContext* ctx);
  int getMethodIndex() {return mMethodIndex;}
  unsigned getNumParameters() {return (unsigned)mSplitSignature.size();}
  VMClass* getParameterClass(VMContext* ctx, int idx);
protected:
  virtual ~VMMethod();
	void parseSignature();

	std::string mName;
	std::string mSignature;
  std::vector<std::string> mSplitSignature;
  std::string mReturnSignature;
	ReturnType mReturnType;
	unsigned mArgSize;
	bool mIsStatic;
	VMClass* mClass;
  int mRefCount;
  int mMethodIndex;
};

class BcVMMethod : public VMMethod{
public:
	BcVMMethod(const std::string& name, const std::string& signature, VMClass* cls, bool isStatic, 
    int methodIndex, Java::Code_attribute* code) : VMMethod(name, signature, cls, isStatic, methodIndex) {mCode = code;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, unsigned ret);
	void executeLongRet(VMContext* ctx);
protected:
	Java::Code_attribute* mCode;
};

class NativeVMMethod : public VMMethod{
public:
	NativeVMMethod(const std::string& name, const std::string& signature, VMClass* cls, bool isStatic, 
    int methodIndex, nativeMethod mthd) : VMMethod(name, signature, cls, isStatic, methodIndex) {mFunction = mthd;}
	void print(std::ostream& strm);
	void execute(VMContext* ctx, unsigned ret);
protected:
  FieldData executeNative(VMContext* ctx, VMMethod::ReturnType ret, VMClass* cls);
  FieldData executeX64(VMContext* ctx, VMMethod::ReturnType ret, VMClass* cls);
  FieldData executeNoASM(VMContext* ctx, VMMethod::ReturnType ret, VMClass* cls);
	nativeMethod mFunction;
};


#endif
