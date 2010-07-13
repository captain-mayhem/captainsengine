#ifndef VMOBJECT_H
#define VM_OBJECT_H

#include "JavaDefs.h"

class VMClass;
class VMContext;

class VMObject{
public:
	VMObject();
	VMObject(VMContext* ctx, VMClass* cls);
	virtual ~VMObject();
	FieldData* getField(unsigned idx);
	VMMethod* getMethod(unsigned idx);
protected:
	std::vector<FieldData> mFields;
	VMClass* mClass;
};

#endif
