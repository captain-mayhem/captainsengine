
#include "Runtime.h"

#include <iostream>

extern "C" {

void Java_java_lang_System_registerNatives(JNIEnv* env, jobject object){
	JNINativeMethod methods[3];
	methods[0].name = "currentTimeMillis";
	methods[0].signature = "()J";
	methods[0].fnPtr = Java_java_lang_System_currentTimeMillis;
	methods[1].name = "nanoTime";
	methods[1].signature = "()J";
	methods[1].fnPtr = Java_java_lang_System_nanoTime;
	methods[2].name = "arraycopy";
	methods[2].signature = "(Ljava/lang/Object;ILjava/lang/Object;II)V";
	methods[2].fnPtr = Java_java_lang_System_arraycopy;
	env->RegisterNatives(object, methods, 3);
}

jlong JNIEXPORT Java_java_lang_System_currentTimeMillis(JNIEnv* env, jobject object){
	return 0;
}

jlong JNIEXPORT Java_java_lang_System_nanoTime(JNIEnv* env, jobject object){
	return 0;
}

void JNIEXPORT Java_java_lang_System_arraycopy(JNIEnv* env, jobject object, jobject src, int srcPos, jobject dest, int destPos, int length){
}

}
