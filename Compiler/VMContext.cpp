#include "VMContext.h"
#include "JVM.h"

VMContext::VMContext(JNIEnv* myself, JVM* vm) : mVm(vm), mSelf(myself){
  JNINativeInterface_::reserved0 = NULL;
  JNINativeInterface_::FindClass = FindClass;
  mVm->addThread(this);
}

VMContext::~VMContext(){
  delete mSelf;
}
