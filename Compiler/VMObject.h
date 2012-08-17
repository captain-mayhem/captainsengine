#ifndef VM_OBJECT_H
#define VM_OBJECT_H

#include "JavaDefs.h"
#include <system/thread.h>

class VMClass;
class VMContext;

class VMObject{
public:
	VMObject(VMClass* cls);
	VMObject(VMContext* ctx, VMClass* cls);
	virtual ~VMObject();
	void init(VMContext* ctx, VMClass* cls);
	FieldData* getObjField(unsigned idx);
	VMMethod* getObjMethod(unsigned idx);
	void lock() {mMuty.lock();}
	void unlock() {mMuty.unlock();}
	VMClass* getClass() {return mClass;}
  void wait() {mCond.wait(mMuty);}
protected:
	std::vector<FieldData> mFields;
	VMClass* mClass;
  CGE::Mutex mMuty;
  CGE::Condition mCond;
};

#endif
