#include "JVM.h"
#include "VMArray.h"
#include "VMLoader.h"

#define CTX(env) ((VMContext*)env->m_func)

TR_CHANNEL(Java_Array);

VMArrayClass::VMArrayClass(VMLoader* loader, const std::string& name) : VMClass(loader){
  mFilename = name;
  mMethodResolver.insert(std::make_pair("clone()Ljava/lang/Object;", 1));
  mMethodResolver.insert(std::make_pair("isArray()Z", 21));
  mMethodResolver.insert(std::make_pair("getComponentType()Ljava/lang/Class;", 35));
  mMethods.resize(35);
  NativeVMMethod* clone = new NativeVMMethod("clone", "()Ljava/lang/Obejct;", this, false, -1, (nativeMethod)cloneFunc);
  mMethods[0] = clone;
  NativeVMMethod* isArray = new NativeVMMethod("isArray", "()Z", this, false, -1, (nativeMethod)isArrayFunc);
  mMethods[20] = isArray;
  NativeVMMethod* getComponentType = new NativeVMMethod("getComponentType", "()Ljava/lang/Class;", this, false, -1, (nativeMethod)getComponentTypeFunc);
  mMethods[34] = getComponentType;
}

jobjectArray VMArrayClass::cloneFunc(JNIEnv* env, jobjectArray array){
  VMArrayBase* ab = (VMArrayBase*)array;
  VMArrayBase* newab = ab->copy(CTX(env));
  return newab;
}

jboolean VMArrayClass::isArrayFunc(JNIEnv* env, jobjectArray array){
  return JNI_TRUE;
}

jclass VMArrayClass::getComponentTypeFunc(JNIEnv* env, jarray array){
  VMArrayBase* ab = (VMArrayBase*)array;
  std::string arrtype = ab->getClass()->getName();
  arrtype.erase(0, 1);
  if (arrtype[0] == 'L'){
    arrtype.erase(0, 1);
    arrtype.erase(arrtype.size()-1);
  }
  VMClass* cls = ab->getClass()->getLoader()->find(CTX(env), arrtype);
  return cls;
}

/*
VMArray::VMArray(int size){
}

VMArray::~VMArray(){
}*/
