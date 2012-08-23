#ifndef VMARRAY_H
#define VMARRAY_H

#include <vector>
#include <cstring>
#include <io/Tracing.h>

#include "VMObject.h"
#include "VMclass.h"

TR_CHANNEL_EXT(Java_Array);

class JVM;
JVM* getVM();

class VMArrayClass : public VMClass{
public:
  VMArrayClass(VMLoader* loader, const std::string& name);
private:
  static jobjectArray cloneFunc(JNIEnv* env, jarray array);
  static jboolean isArrayFunc(JNIEnv* env, jarray array);
  static jclass getComponentTypeFunc(JNIEnv* env, jarray array);
};

class VMArrayBase : public VMObject{
public:
  VMArrayBase(VMContext* ctx, VMClass* cls) : VMObject(ctx, cls) {}
	virtual ~VMArrayBase() {}
	virtual unsigned getLength()=0;
  virtual void* getData(int offset)=0;
  virtual void copyTo(int srcOffset, VMArrayBase* dest, int destOffset, int length)=0;
  virtual VMArrayBase* copy(VMContext* ctx)=0;
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
		return (unsigned)mData.size();
	}
  virtual void* getData(int offset){
    if (mData.empty())
      return NULL;
    return &mData[offset];
  }
  virtual void copyTo(int srcOffset, VMArrayBase* dest, int destOffset, int length){
    void* src = getData(srcOffset);
    void* dst = dest->getData(destOffset);
    memcpy(dst, src, length*sizeof(T));
  }
  virtual VMArrayBase* copy(VMContext* ctx){
    VMArray* newone = new VMArray<T>(ctx, mClass, (unsigned)mData.size());
    ::getVM()->registerObject(newone);
    copyTo(0, newone, 0, (unsigned)mData.size());
    return newone;
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

/* no, no deep copy
template<> VMArrayBase* VMArray<VMObject*>::deepCopy(VMContext* ctx){
  TR_USE(Java_Array);
  VMArray* newone = new VMArray<VMObject*>(ctx, mClass, mData.size());
  if (mData.size() != 0)
    TR_BREAK("Implement deep copy");
  return newone; 
}
*/

typedef VMArray<VMObject*> VMObjectArray;
typedef VMArray<jbyte> VMByteArray;
typedef VMArray<jchar> VMCharArray;
typedef VMArray<jint> VMIntArray;
typedef VMArray<jfloat> VMFloatArray;
typedef VMArray<jdouble> VMDoubleArray;
typedef VMArray<jlong> VMLongArray;

#endif
