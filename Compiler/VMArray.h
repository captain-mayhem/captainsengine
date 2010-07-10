#ifndef VMARRAY_H
#define VMARRAY_H

#include "VMObject.h"

class VMArray : public VMObject{
public:
	VMArray(int size);
	virtual ~VMArray();
};

#endif
