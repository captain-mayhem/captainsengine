#ifndef VMCLASS_H
#define VMCLASS_H

#include <string>
#include <map>
#include "JavaDefs.h"

class VMMethod;
class VMObject;
class VMContext;

class VMClass{
public:
	VMClass(const std::string& filename);
	~VMClass();
	unsigned findMethodIndex(const std::string& name, const std::string& signature);
	unsigned findFieldIndex(const std::string& name);
	FieldData* getField(VMContext* ctx, Java::u2 field_ref);
	FieldData* getField(unsigned fieldid);
	unsigned getFieldIndex(VMContext* ctx, Java::u2 field_ref, VMClass*& classRet);
	unsigned getMethodIndex(VMContext* ctx, Java::u2 method_ref, VMClass*& classRet);
	VMMethod* getMethod(VMContext* ctx, Java::u2 method_ref);
	VMMethod* getMethod(unsigned methodid);
	void print(std::ostream& strm);
	void registerMethod(const std::string& nam, const std::string& signature, nativeMethod mthd);
	VMClass* getClass(VMContext* ctx, Java::u2 class_ref);
	VMClass* getSuperclass(VMContext* ctx);
	StackData getConstant(VMContext* ctx, Java::u2 constant_ref);
	void initFields(VMContext* ctx);
	unsigned getNonStaticFieldOffset();
	void copyMethodData(std::map<std::string,unsigned>& methodresolver, std::vector<VMMethod*>& methods);
protected:
	std::string buildNativeMethodName(const std::string& functionname, const std::string& signature);
	Java::ClassFile mClass;
  std::string mFilename;
	std::map<std::string,unsigned> mMethodResolver;
	std::vector<VMMethod*> mMethods;
	std::map<std::string,unsigned> mFieldResolver;
	std::vector<FieldData> mFields;
	VMClass* mSuperclass;
	std::vector<StackData> mRCP;
	unsigned mVtableEnd;
};

#endif
