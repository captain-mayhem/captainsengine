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
	VMMethod* findMethod(const std::string& name, const std::string& signature);
	unsigned findFieldIndex(const std::string& name);
	FieldData* getField(VMContext* ctx, Java::u2 field_ref);
	unsigned getFieldIndex(VMContext* ctx, Java::u2 field_ref);
	VMMethod* getMethod(VMContext* ctx, Java::u2 method_ref);
	void print(std::ostream& strm);
	void registerMethod(const std::string& nam, const std::string& signature, nativeMethod mthd);
	VMClass* getClass(VMContext* ctx, Java::u2 class_ref);
	VMClass* getSuperclass(VMContext* ctx);
	void initFields(VMContext* ctx);
	unsigned getNonStaticFieldOffset();
protected:
	std::string buildNativeMethodName(const std::string& functionname, const std::string& signature);
	Java::ClassFile mClass;
  std::string mFilename;
	std::map<std::string,VMMethod*> mMethods;
	std::map<std::string,unsigned> mFieldResolver;
	std::vector<FieldData> mFields;
	VMClass* mSuperclass;
	std::vector<StackData> mRCP;
};

#endif
