#include "VMContext.h"
#include "JVM.h"

VMContext::VMContext(JNIEnv* myself, JVM* vm) : mVm(vm), mSelf(myself){
  JNINativeInterface_::reserved0 = NULL;
  JNINativeInterface_::FindClass = FindClass;
	JNINativeInterface_::GetStaticMethodID = GetStaticMethodID;
	JNINativeInterface_::CallStaticVoidMethodV = CallStaticVoidMethodV;
	JNINativeInterface_::NewStringUTF = NewStringUTF;
	JNINativeInterface_::NewObjectArray = NewObjectArray;
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

void VMContext::pushFrame(VMMethod* method, unsigned argsize){
	StackData* oldBase = mBasePointer;
	
	//copy arguments to new frame
	mStackPointer -= argsize;
	memmove(mStackPointer+2, mStackPointer, argsize*sizeof(StackData));

	//setup frame data structure
	*mStackPointer++ = oldBase;
	*mStackPointer++ = method;
	mBasePointer = mStackPointer;

	 //set stack pointer above arguments
	mStackPointer += argsize;
}

void VMContext::popFrame(){
	//jump down the frame data structure
	mBasePointer -= 2;
	StackData* oldBase = mBasePointer->stp;
	mStackPointer = mBasePointer;
	mBasePointer = oldBase;
}

VMMethod* VMContext::getFrameMethod(int numFrames){
	StackData* base = mBasePointer;
	while(numFrames > 0){
		base -= 2;
		base = base->stp;
		--numFrames;
	}
	return (base-1)->mthd;
}
