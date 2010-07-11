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
	//FieldData* getField(VMContext* ctx, Java::u2 field_ref);
protected:
	std::vector<FieldData> mFields;
	VMClass* mClass;
};

#endif
