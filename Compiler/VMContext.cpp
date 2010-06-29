#include "VMContext.h"
#include "JVM.h"

VMContext::VMContext(JNIEnv* myself, JVM* vm) : mVm(vm), mSelf(myself){
  JNINativeInterface_::reserved0 = NULL;
  JNINativeInterface_::FindClass = FindClass;
	JNINativeInterface_::GetStaticMethodID = GetStaticMethodID;
	JNINativeInterface_::CallStaticVoidMethodV = CallStaticVoidMethodV;
	JNINativeInterface_::RegisterNatives = RegisterNatives;
  mVm->addThread(this);
}

VMContext::~VMContext(){
  delete mSelf;
}

//void VMContext::executeFunction(VMClass* cls, VMMethod* method){
//}
