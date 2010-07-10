#ifndef VMOBJECT_H
#define VM_OBJECT_H

class VMClass;

class VMObject{
public:
	VMObject();
	VMObject(VMClass* cls);
	virtual ~VMObject();
};

#endif
