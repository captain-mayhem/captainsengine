#include "VMContext.h"

#include <cstring>

#include "JVM.h"

VMContext::VMContext(JNIEnv* myself, JVM* vm) : mVm(vm), mSelf(myself){
  JNINativeInterface_::reserved0 = NULL;
  JNINativeInterface_::FindClass = FindClass;
	JNINativeInterface_::GetObjectClass = GetObjectClass;
	JNINativeInterface_::GetMethodID = GetMethodID;
	JNINativeInterface_::CallObjectMethodV = CallObjectMethodV;
	JNINativeInterface_::GetFieldID = GetFieldID;
  JNINativeInterface_::SetLongField = SetLongField;
	JNINativeInterface_::GetStaticMethodID = GetStaticMethodID;
	JNINativeInterface_::CallStaticVoidMethodV = CallStaticVoidMethodV;
  JNINativeInterface_::GetStaticFieldID = GetStaticFieldID;
  JNINativeInterface_::SetStaticObjectField = SetStaticObjectField;
	JNINativeInterface_::NewStringUTF = NewStringUTF;
	JNINativeInterface_::GetStringUTFChars = GetStringUTFChars;
	JNINativeInterface_::ReleaseStringUTFChars = ReleaseStringUTFChars;
	JNINativeInterface_::NewObjectArray = NewObjectArray;
	JNINativeInterface_::RegisterNatives = RegisterNatives;
  JNINativeInterface_::NewObjectV = NewObjectV;
  mVm->addThread(this);
	mStack = new StackData[1024];
	mStackPointer = mStack;
	mBasePointer = mStack;
	mThread = vm->createObject(this, NULL);
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

void VMContext::insert(StackData data, int position){
	memmove(mStackPointer-position+1, mStackPointer-position, position*sizeof(StackData));
	*(mStackPointer-position) = data;
	++mStackPointer;
}

void VMContext::dup2(){
	*mStackPointer = *(mStackPointer-2);
	*(mStackPointer+1) = *(mStackPointer-1);
	mStackPointer += 2;
}
