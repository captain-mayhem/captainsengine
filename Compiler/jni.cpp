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
	for (unsigned i = 0; i < mthd->getNumArgs(); ++i){
		VMObject* obj = va_arg(args, VMObject*);
		CTX(env)->push(obj);
	}
	mthd->execute(CTX(env));
}

jint VMContext::RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods){
	VMClass* cls = (VMClass*)clazz;
	for (int i = 0; i < nMethods; ++i){
		cls->registerMethod(methods[i].name, methods[i].signature, (nativeMethod)methods[i].fnPtr);
	}
	return 0;
}

jobjectArray VMContext::NewObjectArray(JNIEnv *env, jsize len, jclass clazz, jobject init){
	return getVM()->createArray(len);
}

JNIEnv_::JNIEnv_(JavaVM_* vm){
  m_func = new VMContext(this, (JVM*)vm->m_func);
}

JNIEnv_::~JNIEnv_(){
  //do nothing here!!!
}
