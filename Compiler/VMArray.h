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
  virtual void* getData(int offset)=0;
  virtual void copyTo(int srcOffset, VMArrayBase* dest, int destOffset, int length)=0;
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
  virtual void* getData(int offset){
    return &mData[offset];
  }
  virtual void copyTo(int srcOffset, VMArrayBase* dest, int destOffset, int length){
    void* src = getData(srcOffset);
    void* dst = dest->getData(destOffset);
    memcpy(dst, src, length*sizeof(T));
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
