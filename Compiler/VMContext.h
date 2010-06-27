#ifndef VMCONTEXT_H
#define VMCONTEXT_H

#include "jni.h"

class JVM;

class VMContext : public JNINativeInterface_{
public:
  VMContext(JNIEnv* myself, JVM* vm);
  ~VMContext();
  JVM* getVM() {return mVm;}
protected:
  static jclass FindClass(JNIEnv* env, const char* name);
	static jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	static void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);

  JVM* mVm;
  JNIEnv* mSelf;
};

#endif
