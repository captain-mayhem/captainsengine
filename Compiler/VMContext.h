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

  JVM* mVm;
  JNIEnv* mSelf;
};

#endif
