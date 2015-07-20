#include "JVM.h"
#include "VMArray.h"
#include "VMLoader.h"

#define CTX(env) ((VMContext*)env->m_func)

TR_CHANNEL(Java_Array);

#ifndef JRE6
#define GET_CLASS_ID 2
#define IS_ARRAY_ID 20
#define GET_COMPONENT_ID 34
#else
define GET_CLASS_ID 2
#define IS_ARRAY_ID 21
#define GET_COMPONENT_ID 35
#endif

VMArrayClass::VMArrayClass(VMLoader* loader, const std::string& name) : VMClass(loader){
  mFilename = name;
  mMethodResolver.insert(std::make_pair("clone()Ljava/lang/Object;", 1));
  mMethodResolver.insert(std::make_pair("getClass()Ljava/lang/Class;", GET_CLASS_ID));
  mMethodResolver.insert(std::make_pair("isArray()Z", IS_ARRAY_ID));
  mMethodResolver.insert(std::make_pair("getComponentType()Ljava/lang/Class;", GET_COMPONENT_ID));
  mMethods.resize(35);
  NativeVMMethod* clone = new NativeVMMethod("clone", "()Ljava/lang/Obejct;", this, false, -1, (nativeMethod)cloneFunc);
  mMethods[0] = clone;
  NativeVMMethod* getClass = new NativeVMMethod("getClass", "()Ljava/lang/Class;", this, false, -1, (nativeMethod)getClassFunc);
  mMethods[GET_CLASS_ID-1] = getClass;
  NativeVMMethod* isArray = new NativeVMMethod("isArray", "()Z", this, false, -1, (nativeMethod)isArrayFunc);
  mMethods[IS_ARRAY_ID-1] = isArray;
  NativeVMMethod* getComponentType = new NativeVMMethod("getComponentType", "()Ljava/lang/Class;", this, false, -1, (nativeMethod)getComponentTypeFunc);
  mMethods[GET_COMPONENT_ID-1] = getComponentType;
}

jobjectArray VMArrayClass::cloneFunc(JNIEnv* env, jobjectArray array){
  VMArrayBase* ab = (VMArrayBase*)array;
  VMArrayBase* newab = ab->copy(CTX(env));
  return newab;
}

jclass VMArrayClass::getClassFunc(JNIEnv* env, jarray array){
  VMArrayBase* ab = (VMArrayBase*)array;
  return ab->getClass();
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
