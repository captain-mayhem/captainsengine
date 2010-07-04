#ifndef VMCONTEXT_H
#define VMCONTEXT_H

#include "jni.h"

class JVM;
class VMClass;
class VMMethod;

class VMContext : public JNINativeInterface_{
public:
	union StackData{
		StackData() {}
		StackData(StackData* sd) {stp = sd;}
		StackData(VMMethod* md) {mthd = md;}
		StackData(uint32 uin) {ui = uin;}
		StackData(void* vop) {vp = vop;}
		VMMethod* mthd;
		StackData* stp;
		int32 i;
		uint32 ui;
		float f;
		void* vp;
	};
  VMContext(JNIEnv* myself, JVM* vm);
  ~VMContext();
	JNIEnv* getJNIEnv() {return mSelf;}
	void pushFrame(VMMethod* method);
	void popFrame();
	void push(StackData data) {*mStackPointer++ = data;}
	StackData pop() {return *--mStackPointer;}
protected:
  static jclass FindClass(JNIEnv* env, const char* name);
	static jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	static void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
	static jint RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods);

  JVM* mVm;
  JNIEnv* mSelf;
	StackData* mStack;
	StackData* mStackPointer;
	StackData* mBasePointer;
};

#endif
