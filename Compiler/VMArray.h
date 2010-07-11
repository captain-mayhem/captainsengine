#ifndef VMARRAY_H
#define VMARRAY_H

#include <vector>

#include "VMObject.h"

class VMArrayBase : public VMObject{
public:
	VMArrayBase() {}
	virtual ~VMArrayBase() {}
	virtual unsigned getLength()=0;
};

template <typename T>
class VMArray : public VMArrayBase{
public:
	VMArray(unsigned size){
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
protected:
	std::vector<T> mData;
};

typedef VMArray<VMObject*> VMObjectArray;
typedef VMArray<char> VMByteArray;

#endif
