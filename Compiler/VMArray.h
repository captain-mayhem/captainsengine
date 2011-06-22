#ifndef VMARRAY_H
#define VMARRAY_H

#include <vector>

#include "VMObject.h"
#include "VMClass.h"

class VMArrayClass : public VMClass{
};

class VMArrayBase : public VMObject{
public:
  VMArrayBase(VMContext* ctx, VMClass* cls) : VMObject(ctx, cls) {}
	virtual ~VMArrayBase() {}
	virtual unsigned getLength()=0;
};

template <typename T>
class VMArray : public VMArrayBase{
public:
  VMArray(VMContext* ctx, VMClass* cls, unsigned size) : VMArrayBase(ctx, cls){
		mData.resize(size);
	}
	virtual ~VMArray(){
	}
	void setData(const T* data){
		memcpy(&mData[0], data, mData.size()*sizeof(T));
	}
	virtual unsigned getLength(){
		return mData.size();
	}
	void put(const T data, unsigned idx){
		mData[idx] = data;
	}
	T get(unsigned idx){
		return mData[idx];
	}
protected:
	std::vector<T> mData;
};

typedef VMArray<VMObject*> VMObjectArray;
typedef VMArray<jbyte> VMByteArray;
typedef VMArray<jchar> VMCharArray;
typedef VMArray<jint> VMIntArray;
typedef VMArray<jfloat> VMFloatArray;
typedef VMArray<jdouble> VMDoubleArray;
typedef VMArray<jlong> VMLongArray;

#endif
