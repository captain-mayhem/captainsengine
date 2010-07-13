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

VMMethod* VMObject::getMethod(unsigned idx){
	return mClass->getMethod(idx);
}
