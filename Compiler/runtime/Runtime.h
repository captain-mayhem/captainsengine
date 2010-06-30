#include <jni.h>

extern "C"{

void JNIEXPORT Java_java_lang_System_registerNatives(JNIEnv* env, jobject object);

jlong JNIEXPORT Java_java_lang_System_currentTimeMillis(JNIEnv* env, jobject object);

jlong JNIEXPORT Java_java_lang_System_nanoTime(JNIEnv* env, jobject object);

void JNIEXPORT Java_java_lang_System_arraycopy(JNIEnv* env, jobject object, jobject src, int srcPos, jobject dest, int destPos, int length);

}
