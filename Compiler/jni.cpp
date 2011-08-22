#include "JVM.h"
#include "VMContext.h"
#include "VMclass.h"
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

jclass VMContext::GetObjectClass(JNIEnv *env, jobject obj){
	VMObject* o = (VMObject*)obj;
	return o->getClass();
}

jmethodID VMContext::GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig){
	VMClass* cls = (VMClass*)clazz;
	return (jmethodID)cls->findMethodIndex(name, sig);
}

jobject VMContext::CallObjectMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args){
	VMObject* o = (VMObject*)obj;
	VMMethod* mthd = o->getObjMethod((unsigned)(jlong)methodID);
	CTX(env)->push(o);
	for (unsigned i = 0; i < mthd->getNumArgs(); ++i){
		VMObject* obj = va_arg(args, VMObject*);
		CTX(env)->push(obj);
	}
	mthd->execute(CTX(env));
	return (jobject)CTX(env)->pop().obj;
}

jfieldID VMContext::GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig){
	VMClass* cls = (VMClass*)clazz;
	return (jfieldID)cls->findFieldIndex(name);
}

jmethodID VMContext::GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig){
	VMClass* cls = (VMClass*)clazz;
	return (jmethodID)cls->findMethodIndex(name, sig);
}

void VMContext::CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args){
	VMClass* cls = (VMClass*)clazz;
	VMMethod* mthd = cls->getMethod((unsigned)(jlong)methodID);
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
  VMClass* cls = (VMClass*)clazz;
	return getVM()->createObjectArray(CTX(env), cls, len);
}

jstring VMContext::NewStringUTF(JNIEnv *env, const char *utf){
	return getVM()->createString(CTX(env), utf);
}

const char* VMContext::GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy){
	if (isCopy)
		*isCopy = JNI_TRUE;
	VMObject* obj = (VMObject*)str;
	VMCharArray* arr = (VMCharArray*)obj->getObjField(obj->getClass()->findFieldIndex("value"))->obj;
	char* data = new char[arr->getLength()+1];
	data[arr->getLength()] = '\0';
	for (unsigned i = 0; i < arr->getLength(); ++i){
		data[i] = (char)arr->get(i);
	}
	return data;
}

void VMContext::ReleaseStringUTFChars(JNIEnv *env, jstring str, const char* chars){
	delete [] chars;
}

jobject VMContext::NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args){
  VMClass* cls = (VMClass*)clazz;
  VMObject* obj = getVM()->createObject(CTX(env), cls);
  CTX(env)->push(obj);
  VMMethod* mthd = cls->getMethod((unsigned)(jlong)methodID);
  mthd->execute(CTX(env));
  return obj;
}

JNIEnv_::JNIEnv_(JavaVM_* vm){
  VMContext* ctx = new VMContext(this, (JVM*)vm->m_func);
	m_func = ctx;
	VMClass* thrdgrpcls = VM_CTX(vm)->findClass(ctx, "java/lang/ThreadGroup");
	VMObject* thrdgrp = VM_CTX(vm)->createObject(ctx, thrdgrpcls);
	unsigned idx = thrdgrpcls->findMethodIndex("<init>", "()V");
	ctx->push(thrdgrp);
	VMMethod* init = thrdgrpcls->getMethod(idx);
	init->execute(ctx);
	//object init without class init
	VMClass* thrdcls = VM_CTX(vm)->defineClass(ctx, "java/lang/Thread");
	ctx->getThread()->init(ctx, thrdcls);
	FieldData* grpfld = ctx->getThread()->getObjField(thrdcls->findFieldIndex("group"));
	grpfld->obj = thrdgrp;
	FieldData* prio = ctx->getThread()->getObjField(thrdcls->findFieldIndex("priority"));
	prio->ui = 5;
	VM_CTX(vm)->findClass(ctx, "java/lang/Thread");
	VM_CTX(vm)->initBasicClasses((VMContext*)m_func);
}

JNIEnv_::~JNIEnv_(){
  //do nothing here!!!
}
