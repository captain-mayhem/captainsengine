#include "JVM.h"
#include "VMArray.h"

#define CTX(env) ((VMContext*)env->m_func)

TR_CHANNEL(Java_Array);

VMArrayClass::VMArrayClass(const std::string& name){
  mFilename = name;
  mMethodResolver.insert(std::make_pair("clone()Ljava/lang/Object;", 1));
  NativeVMMethod* nvmm = new NativeVMMethod("clone", "()Ljava/lang/Obejct;", this, false, (nativeMethod)cloneFunc);
  mMethods.push_back(nvmm);
}

jobjectArray VMArrayClass::cloneFunc(JNIEnv* env, jobjectArray array){
  VMArrayBase* ab = (VMArrayBase*)array;
  VMArrayBase* newab = ab->copy(CTX(env));
  return newab;
}

/*
VMArray::VMArray(int size){
}

VMArray::~VMArray(){
}*/
