#ifndef VMCLASS_H
#define VMCLASS_H

#include <string>
#include <map>
#include "JavaDefs.h"

class VMMethod;
class VMField;
class VMContext;

class VMClass{
public:
	VMClass(const std::string& filename);
	~VMClass();
	VMMethod* findMethod(const std::string& name, const std::string& signature);
	VMField* findField(VMContext* ctx, Java::u2 field_ref);
	VMMethod* getMethod(VMContext* ctx, Java::u2 method_ref);
	void print(std::ostream& strm);
protected:
	Java::ClassFile mClass;
  std::string mFilename;
	std::map<std::string,VMMethod*> mMethods;
};

#endif
