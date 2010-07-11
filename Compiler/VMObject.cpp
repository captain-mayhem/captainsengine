#include "VMObject.h"

#include "VMClass.h"

VMObject::VMObject() : mClass(NULL){
}

VMObject::~VMObject(){
}

VMObject::VMObject(VMContext* ctx, VMClass* cls) : mClass(cls){
	unsigned numFields = 0;
	VMClass* clas = mClass;
	while (clas){
		numFields += clas->getNonStaticFieldOffset();
		clas = clas->getSuperclass(ctx);
	}
	mFields.resize(numFields);
}

FieldData* VMObject::getField(unsigned idx){
	return &mFields[idx-1];
}

/*
FieldData* VMObject::getField(VMContext* ctx, Java::u2 field_ref){
	unsigned idx = 0;
	VMClass* currCls = mClass;
	while(idx == 0){
		idx = currCls->getFieldIndex(ctx, field_ref);
		currCls = mClass->getSuperclass(ctx);
	}
	return NULL;
}
*/
