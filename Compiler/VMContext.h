#ifndef VMCONTEXT_H
#define VMCONTEXT_H

#include "jni.h"
#include "JavaDefs.h"

class JVM;
class VMClass;
class VMMethod;
class VMObject;

class VMContext : public JNINativeInterface_{
public:
  VMContext(JNIEnv* myself, JVM* vm);
  ~VMContext();
	JNIEnv* getJNIEnv() {return mSelf;}
	void pushFrame(VMMethod* method, unsigned argsize);
	void popFrame();
	void push(StackData data) {*mStackPointer++ = data;}
	void insert(StackData data, int position);
	void dup() {*mStackPointer++ = *(mStackPointer-1);}
	void dup2();
	StackData get(int idx) {return mBasePointer[idx];}
	void put(int idx, StackData data) {mBasePointer[idx] = data;}
	StackData getTop(int idx) {return *(mStackPointer-idx-1);}
	StackData pop() {return *--mStackPointer;}
	VMMethod* getFrameMethod(int numFrames);
	VMObject* getThread() {return mThread;}
protected:
  static jclass FindClass(JNIEnv* env, const char* name);
  static jclass GetSuperclass(JNIEnv *env, jclass sub);
	static jclass GetObjectClass(JNIEnv *env, jobject obj);
	static jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	static jobject CallObjectMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
	static jobject NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
  static jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
  static void SetLongField(JNIEnv *env, jobject obj, jfieldID fieldID, jlong value);
	static jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	static void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
	static jfieldID GetStaticFieldID(JNIEnv* env, jclass clazz, const char *name, const char *sig);
  static void SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value);
  static jstring NewStringUTF(JNIEnv *env, const char *utf);
	static const char* GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy);
  static void ReleaseStringUTFChars(JNIEnv *env, jstring str, const char* chars);
	static jobjectArray NewObjectArray(JNIEnv *env, jsize len, jclass clazz, jobject init);
  static void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject val);
	static jint RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods);

  JVM* mVm;
  JNIEnv* mSelf;
	StackData* mStack;
	StackData* mStackPointer;
	StackData* mBasePointer;
	VMObject* mThread;
};

#endif
