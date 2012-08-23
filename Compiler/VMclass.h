#ifndef VMCLASS_H
#define VMCLASS_H

#include <string>
#include <map>
#include <io/Reader.h>
#include "JavaDefs.h"
#include "VMObject.h"
#include "VMMethod.h"

class VMMethod;
class VMContext;
class VMLoader;

class VMClass : public VMObject{
public:
	VMClass(VMLoader* loader);
  VMClass(VMContext* ctx, VMLoader* loader, CGE::Reader& reader);
	virtual ~VMClass();
  void initClass(VMContext* ctx, bool execClassInit);
	unsigned findMethodIndex(const std::string& name, const std::string& signature);
	unsigned findFieldIndex(const std::string& name);
	FieldData* getField(VMContext* ctx, Java::u2 field_ref, VMMethod::ReturnType& type);
  FieldData* getStaticField(VMContext* ctx, Java::u2 field_ref, VMMethod::ReturnType& type);
	FieldData* getField(unsigned fieldid);
  FieldData* getStaticField(unsigned fieldid);
	unsigned getFieldIndex(VMContext* ctx, Java::u2 field_ref, VMClass*& classRet, VMMethod::ReturnType& type);
  unsigned getStaticFieldIndex(VMContext* ctx, Java::u2 field_ref, VMClass*& classRet, VMMethod::ReturnType& type);
	unsigned getMethodIndex(VMContext* ctx, Java::u2 method_ref, VMClass*& classRet);
	VMMethod* getMethod(VMContext* ctx, Java::u2 method_ref);
	VMMethod* getMethod(unsigned methodid);
	void print(std::ostream& strm);
	void registerMethod(const std::string& nam, const std::string& signature, nativeMethod mthd);
	VMClass* getClass(VMContext* ctx, Java::u2 class_ref);
	VMClass* getSuperclass(VMContext* ctx);
	FieldData getConstant(VMContext* ctx, Java::u2 constant_ref);
	unsigned getNonStaticFieldOffset();
  unsigned getStaticFieldOffset();
	void copyMethodData(std::map<std::string,unsigned>& methodresolver, std::vector<VMMethod*>& methods);
  void copyFieldData(std::map<std::string,unsigned>& fieldresolver, std::map<unsigned,VMClass*>& staticfieldresolver);
	Java::ClassFile& getClassDefinition() {return mClass;}
	VMObject* getClassObject() {return this;}
	std::string getName() {return mFilename;}
  void setName(const std::string& str) {mFilename = str;}
  std::string getSourceFileName();
  int getLineNumber(int ip, int methodIndex);
  int getCatchIP(int ip, VMContext* ctx, VMObject* exception, int methodIndex);
  VMLoader* getLoader() {return mLoader;}
protected:
  void initFields(VMContext* ctx);
	std::string buildNativeMethodName(const std::string& functionname, const std::string& signature);
	Java::ClassFile mClass;
  std::string mFilename;
	std::map<std::string,unsigned> mMethodResolver;
	std::vector<VMMethod*> mMethods;
	std::map<std::string,unsigned> mFieldResolver;
  std::map<unsigned, VMClass*> mStaticFieldResolver;
	std::vector<FieldData> mFields;
	std::vector<StackData> mRCP;
	unsigned mVtableEnd;
  unsigned mSuperFields;
  unsigned mNonStaticFieldOffset;
	VMLoader* mLoader;
};

#endif
