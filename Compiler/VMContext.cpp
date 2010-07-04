#include "VMContext.h"
#include "JVM.h"

VMContext::VMContext(JNIEnv* myself, JVM* vm) : mVm(vm), mSelf(myself){
  JNINativeInterface_::reserved0 = NULL;
  JNINativeInterface_::FindClass = FindClass;
	JNINativeInterface_::GetStaticMethodID = GetStaticMethodID;
	JNINativeInterface_::CallStaticVoidMethodV = CallStaticVoidMethodV;
	JNINativeInterface_::RegisterNatives = RegisterNatives;
  mVm->addThread(this);
	mStack = new StackData[1024];
	mStackPointer = mStack;
	mBasePointer = mStack;
}

VMContext::~VMContext(){
  delete mSelf;
	delete [] mStack;
}

void VMContext::pushFrame(VMMethod* method){
	StackData* oldBase = mBasePointer;
	mBasePointer = mStackPointer-0;
	*mStackPointer++ = oldBase;
	*mStackPointer++ = method;
}

void VMContext::popFrame(){
	StackData* oldBase = mBasePointer->stp;
	mStackPointer = mBasePointer;
	mBasePointer = oldBase;
}
