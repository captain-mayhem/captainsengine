#ifndef VMCONTEXT_H
#define VMCONTEXT_H

#include "jni.h"
#include "JavaDefs.h"

class JVM;
class VMClass;
class VMMethod;
class VMObject;
class VMLoader;

class VMContext : public JNINativeInterface_{
public:
  class StackIterator{
    friend class VMContext;
  public:
    bool hasNext();
    StackIterator& next();
    VMMethod* getMethod();
    unsigned getReturnIP();
  private:
    StackIterator(VMContext* ctx, StackData* data) : mCtx(ctx), mCurrBasePointer(data) {}
    VMContext* mCtx;
    StackData* mCurrBasePointer;
  };
  VMContext(JNIEnv* myself, JVM* vm, VMObject* thrd);
  ~VMContext();
  void setJNIEnv(JNIEnv* env) {mSelf = env;}
	JNIEnv* getJNIEnv() {return mSelf;}
	void pushFrame(VMMethod* method, unsigned ip, unsigned argsize);
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
  StackIterator getTopFrame();
	VMObject* getThread() {return mThread;}
  void throwException(VMObject* exception) {mException = exception;}
  VMObject* getException() {return mException;}
protected:
  static jclass FindClass(JNIEnv* env, const char* name);
  static jclass GetSuperclass(JNIEnv *env, jclass sub);
  static jboolean IsAssignableFrom(JNIEnv *env, jclass sub, jclass sup);
  static jint Throw(JNIEnv *env, jthrowable obj);
  static jint ThrowNew(JNIEnv *env, jclass clazz, const char *msg);
  static jthrowable ExceptionOccurred(JNIEnv *env);
  static void ExceptionDescribe(JNIEnv *env);
  static void ExceptionClear(JNIEnv *env);
	static jclass GetObjectClass(JNIEnv *env, jobject obj);
	static jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	static jobject CallObjectMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
  static jint CallIntMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
  static void CallVoidMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
  static jobject NewObject(JNIEnv *env, jclass clazz, jmethodID methodID, ...);
	static jobject NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
  static jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
  static jobject GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID);
  static jint GetIntField(JNIEnv *env, jobject obj, jfieldID fieldID);
  static jlong GetLongField(JNIEnv *env, jobject obj, jfieldID fieldID);
  static void SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, jobject val);
  static void SetLongField(JNIEnv *env, jobject obj, jfieldID fieldID, jlong value);
	static jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
	static void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
	static jfieldID GetStaticFieldID(JNIEnv* env, jclass clazz, const char *name, const char *sig);
  static void SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value);
  static jstring NewStringUTF(JNIEnv *env, const char *utf);
	static const char* GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy);
  static void ReleaseStringUTFChars(JNIEnv *env, jstring str, const char* chars);
  static jsize GetArrayLength(JNIEnv *env, jarray array);
	static jobjectArray NewObjectArray(JNIEnv *env, jsize len, jclass clazz, jobject init);
  static jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index);
  static void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject val);
  static jbyte * GetByteArrayElements(JNIEnv *env, jbyteArray array, jboolean *isCopy);
  static void ReleaseByteArrayElements(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode);
	static jint RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods);

  JVM* mVm;
  JNIEnv* mSelf;
	StackData* mStack;
	StackData* mStackPointer;
	StackData* mBasePointer;
	VMObject* mThread;
  VMObject* mException;
  VMLoader* mCurrentLoader;
};

#endif
