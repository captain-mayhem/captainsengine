#include "VMContext.h"

#include <cstring>

#include "JVM.h"
#include "VMObject.h"
#include "VMclass.h"
#include "VMLoader.h"

VMContext::VMContext(JNIEnv* myself, JVM* vm, VMObject* thrd) : mVm(vm), mSelf(myself), mException(NULL){
  JNINativeInterface_::reserved0 = NULL;
  JNINativeInterface_::FindClass = FindClass;
  JNINativeInterface_::GetSuperclass = GetSuperclass;
  JNINativeInterface_::IsAssignableFrom = IsAssignableFrom;
  JNINativeInterface_::Throw = Throw;
  JNINativeInterface_::ThrowNew = ThrowNew;
  JNINativeInterface_::ExceptionDescribe = ExceptionDescribe;
  JNINativeInterface_::ExceptionOccurred = ExceptionOccurred;
  JNINativeInterface_::ExceptionClear = ExceptionClear;
	JNINativeInterface_::GetObjectClass = GetObjectClass;
	JNINativeInterface_::GetMethodID = GetMethodID;
	JNINativeInterface_::CallObjectMethodV = CallObjectMethodV;
  JNINativeInterface_::CallIntMethodV = CallIntMethodV;
  JNINativeInterface_::CallVoidMethodV = CallVoidMethodV;
  JNINativeInterface_::NewObject = NewObject;
	JNINativeInterface_::GetFieldID = GetFieldID;
  JNINativeInterface_::GetObjectField = GetObjectField;
  JNINativeInterface_::GetIntField = GetIntField;
  JNINativeInterface_::GetLongField = GetLongField;
  JNINativeInterface_::SetObjectField = SetObjectField;
  JNINativeInterface_::SetLongField = SetLongField;
	JNINativeInterface_::GetStaticMethodID = GetStaticMethodID;
	JNINativeInterface_::CallStaticVoidMethodV = CallStaticVoidMethodV;
  JNINativeInterface_::GetStaticFieldID = GetStaticFieldID;
  JNINativeInterface_::SetStaticObjectField = SetStaticObjectField;
	JNINativeInterface_::NewStringUTF = NewStringUTF;
	JNINativeInterface_::GetStringUTFChars = GetStringUTFChars;
	JNINativeInterface_::ReleaseStringUTFChars = ReleaseStringUTFChars;
  JNINativeInterface_::GetArrayLength = GetArrayLength;
	JNINativeInterface_::NewObjectArray = NewObjectArray;
  JNINativeInterface_::GetObjectArrayElement = GetObjectArrayElement;
  JNINativeInterface_::SetObjectArrayElement = SetObjectArrayElement;
  JNINativeInterface_::GetByteArrayElements = GetByteArrayElements;
  JNINativeInterface_::ReleaseByteArrayElements = ReleaseByteArrayElements;
	JNINativeInterface_::RegisterNatives = RegisterNatives;
  JNINativeInterface_::NewObjectV = NewObjectV;
  mVm->addThread(this);
	mStack = new StackData[1024];
	mStackPointer = mStack;
	mBasePointer = mStack;
  if (thrd == NULL)
    mThread = vm->createObject(this, NULL);
  else
	  mThread = thrd;
  mCurrentLoader = NULL;
}

VMContext::~VMContext(){
  VMClass* thrdcls = mVm->getLoader(NULL)->find(this, "java/lang/Thread");
  FieldData* eetop = mThread->getObjField(thrdcls->findFieldIndex("eetop"));
  CGE::Thread* thrd = (CGE::Thread*)eetop->l;
  delete thrd;
  delete mSelf;
	delete [] mStack;
}

#define FRAME_METADATA 3

void VMContext::pushFrame(VMMethod* method, unsigned ip, unsigned argsize){
	StackData* oldBase = mBasePointer;
	
	//copy arguments to new frame
	mStackPointer -= argsize;
	memmove(mStackPointer+FRAME_METADATA, mStackPointer, argsize*sizeof(StackData));

	//setup frame data structure
	*mStackPointer++ = oldBase;
  *mStackPointer++ = ip;
	*mStackPointer++ = method;
	mBasePointer = mStackPointer;

	 //set stack pointer above arguments
	mStackPointer += argsize;
}

void VMContext::popFrame(){
	//jump down the frame data structure
	mBasePointer -= FRAME_METADATA;
	StackData* oldBase = mBasePointer->stp;
	mStackPointer = mBasePointer;
	mBasePointer = oldBase;
}

VMMethod* VMContext::getFrameMethod(int numFrames){
	StackData* base = mBasePointer;
	while(numFrames > 0){
		base -= FRAME_METADATA;
    if (base <= mStack)
      return NULL;
		base = base->stp;
		--numFrames;
	}
  //if (base-1 < mStack)
  //  return 0;
	return (base-1)->mthd;
}

VMContext::StackIterator VMContext::getTopFrame(){
  return StackIterator(this, mBasePointer);
}

bool VMContext::StackIterator::hasNext(){
  return mCurrBasePointer - FRAME_METADATA > mCtx->mStack;
}

VMContext::StackIterator& VMContext::StackIterator::next(){
  mCurrBasePointer -= FRAME_METADATA;
  mCurrBasePointer = mCurrBasePointer->stp;
  return *this;
}

VMMethod* VMContext::StackIterator::getMethod(){
  if (mCurrBasePointer == mCtx->mStack)
    return NULL;
  return (mCurrBasePointer-1)->mthd;
}

unsigned VMContext::StackIterator::getReturnIP(){
  return (mCurrBasePointer-2)->ui;
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
