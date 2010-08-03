
#include "Runtime.h"

#include <VMContext.h>
#include <VMMethod.h>
#include <VMClass.h>
#include <JVM.h>
#include <Trace.h>

#include <iostream>

#define CTX(env) ((VMContext*)env->m_func)

extern "C" {

void JNIEXPORT Java_java_lang_Class_registerNatives(JNIEnv* env, jobject object){
	return;
}

jboolean JNIEXPORT Java_java_lang_Class_desiredAssertionStatus0(JNIEnv* env, jobject object, jobject cls){
	return 0;
}

jobject JNIEXPORT Java_java_lang_Class_getClassLoader0(JNIEnv* env, jobject object){
	return NULL;
}

jobjectArray JNIEXPORT Java_java_lang_Class_getDeclaredFields0(JNIEnv* env, jobject object, jboolean publicOnly){
	if (publicOnly){
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "publicOnly not implemented");
	}
	VMContext* ctx = CTX(env);
	VMObject* obj = (VMObject*)object;
	Java::ClassFile& cls = obj->getClass()->getClassDefinition();

	VMClass* fieldcls = getVM()->findClass(CTX(env), "java/lang/reflect/Field");
	VMObjectArray* arr = getVM()->createObjectArray(cls.fields_count);
	arr->init(CTX(env), fieldcls);

	for (unsigned i = 0; i < cls.fields_count; ++i){
		Java::field_info* info = cls.fields[i];
		std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
		VMObject* arrobj = getVM()->createObject(CTX(env), fieldcls);
		CTX(env)->push(arrobj);
		CTX(env)->push(obj->getClass()->getClassObject());
		CTX(env)->push(obj->getClass()->getConstant(CTX(env), info->name_index).ui);
		ctx->push(0u); //type class
		ctx->push(info->access_flags);
		ctx->push(obj->getClass()->findFieldIndex(name));
		ctx->push(obj->getClass()->getConstant(ctx, info->descriptor_index).ui);
		ctx->push(0u);
		VMMethod* mthd = fieldcls->getMethod(fieldcls->findMethodIndex("<init>", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;IILjava/lang/String;[B)V"));
		mthd->execute(ctx);
		arr->put(arrobj, i);
	}
	
	return arr;
}

jclass JNIEXPORT Java_java_lang_Class_getPrimitiveClass(JNIEnv* env, jclass cls, jstring name){
	const char* namestr = env->GetStringUTFChars(name, NULL);
	VMClass* clazz = NULL;
	if (strcmp(namestr, "float") == 0){
		clazz = getVM()->getPrimitiveClass(CTX(env), "F");
	}
	else if (strcmp(namestr, "double") == 0){
		clazz = getVM()->getPrimitiveClass(CTX(env), "D");
	}
	else{
		TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "getPrimitiveClass not implemented for this type");
	}
	env->ReleaseStringUTFChars(name, namestr);
	return clazz;
}

void JNIEXPORT Java_java_lang_ClassLoader_registerNatives(JNIEnv* env, jobject object){
	return;
}

jlong JNIEXPORT Java_java_lang_Double_doubleToRawLongBits(JNIEnv* env, jobject object, jdouble value){
	return *((jlong*)(&value));
}

jint JNIEXPORT Java_java_lang_Float_floatToRawIntBits(JNIEnv* env, jobject object, jfloat value){
	return *((jint*)(&value));
}

void JNIEXPORT Java_java_lang_Object_registerNatives(JNIEnv* env, jobject object){
	return;
}

jint JNIEXPORT Java_java_lang_Object_hashCode(JNIEnv* env, jobject object){
	return (jint)object;
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
#ifdef WIN32
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return (jlong)(counter.QuadPart*1000000000/(double)frequency.QuadPart);
#endif
	return 0;
}

void JNIEXPORT Java_java_lang_System_arraycopy(JNIEnv* env, jobject object, jobject src, int srcPos, jobject dest, int destPos, int length){
	TRACE(TRACE_JAVA, TRACE_FATAL_ERROR, "arraycopy not implemented");
}

void JNIEXPORT Java_java_lang_Thread_registerNatives(JNIEnv* env, jobject object){
}

jobject JNIEXPORT Java_java_lang_Thread_currentThread(JNIEnv* env, jclass clazz){
	return CTX(env)->getThread();
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
