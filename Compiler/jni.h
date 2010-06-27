#ifndef JNI_H
#define JNI_H

#ifdef __cplusplus
#include <fstream>
#include <cstdarg>
#else
#include <stdio.h>
#include <stdarg.h>
#endif

typedef int jint;
typedef void* jclass;
typedef void* jmethodID;

typedef struct JDK1_1InitArgs{
  jint version;
  char** properties;
  jint checkSource;
  jint nativeStackSize;
  jint minHeapSize;
  jint verifyMode;
  const char* classpath;
  jint (*vfprintf)(FILE *fp, const char *format, va_list args);
  void (*exit)(jint code);
  void (*abort)();
  jint enableClassGC;
  jint enableVerboseGC;
  jint disableAsyncGC;
  jint reserved0;
  jint reserved1;
  jint reserved2;
} JDK1_1InitArgs;

#ifdef __cplusplus
class JavaVM_;
typedef JavaVM_ JavaVM;
#else
typedef const struct JNIInvokeInterface_ *JavaVM;
#endif

struct JNIInvokeInterface_{
  void* reserved0;
  jint (*DestroyJavaVM)(JavaVM *vm);
};

#ifdef __cplusplus
class JNIEnv_;
typedef JNIEnv_ JNIEnv;
#else
typedef const struct JNINativeInterface_ *JNIEnv;
#endif

struct JNINativeInterface_{
  void* reserved0;
  jclass (*FindClass)(JNIEnv *env, const char* name);
	jmethodID (*GetStaticMethodID)(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	void (*CallStaticVoidMethodV)(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
};

jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);

jint JNI_GetDefaultJavaVMInitArgs(void *vm_args);

#ifdef __cplusplus
class JavaVM_{
  friend jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);
public:
  JNIInvokeInterface_* m_func;
  ~JavaVM_();
  jint DestroyJavaVM() {return m_func->DestroyJavaVM(this);}
protected:
  JavaVM_();
};

class JNIEnv_{
  friend jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);
public:
  JNINativeInterface_* m_func;
  ~JNIEnv_();
  jclass FindClass(const char* name) {return m_func->FindClass(this, name);}
	jmethodID GetStaticMethodID(jclass clazz, const char* name, const char* sig) {return m_func->GetStaticMethodID(this, clazz, name, sig);}
	void CallStaticVoidMethod(jclass clazz, jmethodID methodID, ...) {va_list args; va_start(args, methodID); m_func->CallStaticVoidMethodV(this, clazz, methodID, args); va_end(args);}
protected:
  JNIEnv_(JavaVM_* vm);
};
#endif

#endif
