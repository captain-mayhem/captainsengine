
#include "Runtime.h"

#include <VMContext.h>
#include <VMMethod.h>
#include <VMClass.h>

#include <iostream>

#define CTX(env) ((VMContext*)env->m_func)

extern "C" {

void JNIEXPORT Java_java_lang_Class_registerNatives(JNIEnv* env, jobject object){
	return;
}

jobject JNIEXPORT Java_java_lang_Class_getClassLoader0(JNIEnv* env, jobject object){
	return NULL;
}

jobjectArray JNIEXPORT Java_java_lang_Class_getDeclaredFields0(JNIEnv* env, jobject object, jboolean unknown){
	return NULL;
}

jboolean JNIEXPORT Java_java_lang_Class_desiredAssertionStatus0(JNIEnv* env, jobject object, jobject cls){
	return 0;
}

void JNIEXPORT Java_java_lang_ClassLoader_registerNatives(JNIEnv* env, jobject object){
	return;
}

void Java_java_lang_Object_registerNatives(JNIEnv* env, jobject object){
	return;
}

void Java_java_lang_System_registerNatives(JNIEnv* env, jobject object){
	JNINativeMethod methods[3];
	methods[0].name = (char*)"currentTimeMillis";
	methods[0].signature = (char*)"()J";
	methods[0].fnPtr = (void*)Java_java_lang_System_currentTimeMillis;
	methods[1].name = (char*)"nanoTime";
	methods[1].signature = (char*)"()J";
	methods[1].fnPtr = (void*)Java_java_lang_System_nanoTime;
	methods[2].name = (char*)"arraycopy";
	methods[2].signature = (char*)"(Ljava/lang/Object;ILjava/lang/Object;II)V";
	methods[2].fnPtr = (void*)Java_java_lang_System_arraycopy;
	env->RegisterNatives(object, methods, 3);
}

#ifdef WIN32
static int64 time1970=0;
#endif

jlong JNIEXPORT Java_java_lang_System_currentTimeMillis(JNIEnv* env, jobject object){
#ifdef WIN32
	SYSTEMTIME systime;
		FILETIME filetime;
	if (time1970 == 0){
		systime.wDay = 1;
		systime.wDayOfWeek = 4;
		systime.wHour = 0;
		systime.wMilliseconds = 0;
		systime.wMinute = 0;
		systime.wMonth = 1;
		systime.wSecond = 0;
		systime.wYear = 1970;
		SystemTimeToFileTime(&systime, &filetime);
		time1970 = (int64)filetime.dwHighDateTime << 32 | filetime.dwLowDateTime;
	}
	GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, &filetime);
	int64 time = (int64)filetime.dwHighDateTime << 32 | filetime.dwLowDateTime;
	return (time - time1970)/10000;
#endif
	return 0;
}

jlong JNIEXPORT Java_java_lang_System_nanoTime(JNIEnv* env, jobject object){
	return 0;
}

void JNIEXPORT Java_java_lang_System_arraycopy(JNIEnv* env, jobject object, jobject src, int srcPos, jobject dest, int destPos, int length){
}

jobject JNIEXPORT Java_java_security_AccessController_doPrivileged(JNIEnv* env, jobject object, jobject action){
	VMObject* obj = (VMObject*)action;
	VMMethod* mthd = obj->getObjMethod(obj->getClass()->findMethodIndex("run", "()Ljava/lang/Object;"));
	CTX(env)->push(obj);
	mthd->execute(CTX(env));
	obj = CTX(env)->pop().obj;
	return obj;
}

void JNIEXPORT Java_sun_misc_Unsafe_registerNatives(JNIEnv* env, jobject object){
	return;
}

jobject JNIEXPORT Java_sun_reflect_Reflection_getCallerClass(JNIEnv* env, jobject object, jint realFramesToSkip){
	VMContext* ctx = CTX(env);
	VMMethod* mthd = ctx->getFrameMethod(realFramesToSkip);
	return (VMObject*)mthd->getClass();
}

}
