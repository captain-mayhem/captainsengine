#ifndef VM_OBJECT_H
#define VM_OBJECT_H

#include "JavaDefs.h"

class VMClass;
class VMContext;

class VMObject{
public:
	VMObject();
	VMObject(VMContext* ctx, VMClass* cls);
	virtual ~VMObject();
	void init(VMContext* ctx, VMClass* cls);

	FieldData* getObjField(unsigned idx);
	VMMethod* getObjMethod(unsigned idx);
	void lock() {}
	void unlock() {}
	VMClass* getClass() {return mClass;}
protected:
	std::vector<FieldData> mFields;
	VMClass* mClass;
};

#endif
