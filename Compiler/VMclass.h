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
	VMObject** findField(const std::string& name);
	VMObject** getField(VMContext* ctx, Java::u2 field_ref);
	VMMethod* getMethod(VMContext* ctx, Java::u2 method_ref, VMClass*& classRet);
	void print(std::ostream& strm);
	void registerMethod(const std::string& nam, const std::string& signature, nativeMethod mthd);
	VMClass* getClass(VMContext* ctx, Java::u2 class_ref);
	VMClass* getSuperclass(VMContext* ctx);
protected:
	std::string buildNativeMethodName(const std::string& functionname, const std::string& signature);
	Java::ClassFile mClass;
  std::string mFilename;
	std::map<std::string,VMMethod*> mMethods;
	std::map<std::string,VMObject*> mFields;
	VMClass* mSuperclass;
};

#endif
