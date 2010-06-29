#include "JVM.h"
#include "VMContext.h"
#include "VMClass.h"
#include "VMMethod.h"

#define CTX(env) ((VMContext*)env->m_func)
#define VM_CTX(vm) ((JVM*)vm->m_func)

jint JNI_GetDefaultJavaVMInitArgs(void *vm_args){
  JDK1_1InitArgs* args = (JDK1_1InitArgs*)vm_args;
  if (args->version != 0x10001000)
    return -1;
  return 0;
}

jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args){
  *p_vm = new JavaVM();
  *p_env = new JNIEnv(*p_vm);
  return 0;
}



jint JVM::DestroyJavaVM(JavaVM* vm){
  delete vm;
  return 0;
}

JavaVM_::JavaVM_(){
  m_func = new JVM();
}

JavaVM_::~JavaVM_(){
  delete ((JVM*)m_func);
}

jclass VMContext::FindClass(JNIEnv* env, const char* name){
  return getVM()->findClass(CTX(env), name);
}

jmethodID VMContext::GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig){
	VMClass* cls = (VMClass*)clazz;
	return cls->findMethod(name, sig);
}

void VMContext::CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args){
	VMClass* cls = (VMClass*)clazz;
	VMMethod* mthd = (VMMethod*)methodID;
	mthd->execute(CTX(env), cls);
}

jint VMContext::RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods){
	return 0;
}

JNIEnv_::JNIEnv_(JavaVM_* vm){
  m_func = new VMContext(this, (JVM*)vm->m_func);
}

JNIEnv_::~JNIEnv_(){
  //do nothing here!!!
}
