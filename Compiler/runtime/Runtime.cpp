
#include "Runtime.h"

#ifdef WIN32
#include <direct.h>
#endif

#include <VMContext.h>
#include <VMMethod.h>
#include <VMclass.h>
#include <JVM.h>
#include <Trace.h>
#include <system/utilities.h>

#include <iostream>

TR_CHANNEL(Java_Runtime);

#define CTX(env) ((VMContext*)env->m_func)

extern "C" {

void JNIEXPORT Java_java_io_FileDescriptor_initIDs(JNIEnv* env, jobject object){
  return;
}

jlong JNIEXPORT Java_java_io_FileDescriptor_set(JNIEnv* env, jobject object, jint fd){
  FILE* handle = NULL;
  if (fd == 0)
    handle = stdin;
  else if (fd == 1)
    handle = stdout;
  else if (fd == 2)
    handle = stderr;
  return (jlong)handle;
}

void JNIEXPORT Java_java_io_FileInputStream_initIDs(JNIEnv* env, jobject object){
  return;
}

void JNIEXPORT Java_java_io_FileOutputStream_initIDs(JNIEnv* env, jobject object){
  return;
}

jobject JNIEXPORT Java_java_io_FileSystem_getFileSystem(JNIEnv* env, jobject object){
  jclass fs = env->FindClass("java/io/WinNTFileSystem");
  jmethodID constr = env->GetMethodID(fs, "<init>", "()V");
  jobject ret = env->NewObject(fs, constr);
  return ret;
}

void JNIEXPORT Java_java_io_Win32FileSystem_initIDs(JNIEnv* env, jobject object){
}

jstring JNIEXPORT Java_java_io_WinNTFileSystem_canonicalize0(JNIEnv* env, jobject object, jstring filename){
  const char* str = env->GetStringUTFChars(filename, NULL);
  std::string name = std::string(str);
  jstring ret = env->NewStringUTF(name.c_str());
	env->ReleaseStringUTFChars(filename, str);
  return ret;
}

jint JNIEXPORT Java_java_io_WinNTFileSystem_getBooleanAttributes(JNIEnv* env, jobject object, jobject file){
  jclass filecls = env->GetObjectClass(file);
  jmethodID getAbsPath = env->GetMethodID(filecls, "getAbsolutePath", "()Ljava/lang/String;");
  jstring path = env->CallObjectMethod(file, getAbsPath);
  const char* str = env->GetStringUTFChars(path, NULL);
#ifdef WIN32
  DWORD attribs = GetFileAttributes(str);
#endif
  env->ReleaseStringUTFChars(path, str);
  int jattribs = 0;
#ifdef WIN32
  if (attribs == INVALID_FILE_ATTRIBUTES)
    return jattribs;
  jattribs |= 0x01; //exists
  if (attribs & FILE_ATTRIBUTE_DIRECTORY)
    jattribs |= 0x04; //directory
  else
    jattribs |= 0x02; //regular
  if (attribs & FILE_ATTRIBUTE_HIDDEN)
    jattribs |= 0x08; //hidden
#endif
  return jattribs;
}

void JNIEXPORT Java_java_io_WinNTFileSystem_initIDs(JNIEnv* env, jobject object){
}

void JNIEXPORT Java_java_lang_Class_registerNatives(JNIEnv* env, jobject object){
	return;
}

jclass JNIEXPORT Java_java_lang_Class_forName0(JNIEnv* env, jclass clazz, jstring name, jboolean initialize, jobject classloader){
  const char* namestr = env->GetStringUTFChars(name, NULL);
  std::string str = namestr;
  CGE::Utilities::replaceWith(str, '.', '/');
  jclass retclass = NULL;
  if (initialize == JNI_TRUE){
    retclass = env->FindClass(str.c_str());
  }
  else{
    retclass = getVM()->defineClass(CTX(env), str);
  }
  env->ReleaseStringUTFChars(name, namestr);
  return retclass;
}

jboolean JNIEXPORT Java_java_lang_Class_desiredAssertionStatus0(JNIEnv* env, jobject object, jobject cls){
	return 0;
}

jobject JNIEXPORT Java_java_lang_Class_getClassLoader0(JNIEnv* env, jobject object){
	return NULL;
}

jobjectArray JNIEXPORT Java_java_lang_Class_getDeclaredConstructors0(JNIEnv* env, jobject object, jboolean publicOnly){
  TR_USE(Java_Runtime);
  if (publicOnly){
    TR_BREAK("publicOnly not implemented");
  }
  //VMContext* ctx = CTX(env);
  //VMObject* obj = (VMObject*)object;
  VMClass* objcls = (VMClass*)object;
  Java::ClassFile& cls = objcls->getClassDefinition();

  int constructorcount = 0;
  for (int i = 0; i < cls.methods_count; ++i){
    Java::method_info* info = cls.methods[i];
    std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
    if (name == "<init>")
      ++constructorcount;
    //std::string  sig = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->descriptor_index-1]))->bytes;
    //continue;
  }

  jclass constructorcls = env->FindClass("java/lang/reflect/Constructor");
  jmethodID constr = env->GetMethodID(constructorcls, "<init>", "(Ljava/lang/Class;[Ljava/lang/Class;[Ljava/lang/Class;IILjava/lang/String;[B[B)V");
  jobjectArray ret = env->NewObjectArray(constructorcount, constructorcls, NULL);

  constructorcount = 0;
  for (int i = 0; i < cls.methods_count; ++i){
    Java::method_info* info = cls.methods[i];
    std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
    if (name != "<init>")
      continue;
    std::string  sig = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->descriptor_index-1]))->bytes;
    unsigned id = objcls->findMethodIndex(name, sig);
    VMMethod* mthd = objcls->getMethod(id);
    jclass theobjectclass = env->FindClass("java/lang/Class");
    jobjectArray parametertypes = env->NewObjectArray(mthd->getNumArgs(), theobjectclass, NULL);
    for (unsigned i = 0; i < mthd->getNumArgs(); ++i){
      TR_BREAK("Parameters unimplemented");
    }
    //TODO
    jobjectArray checkedExceptions = env->NewObjectArray(0, theobjectclass, NULL);
    jobject constrobj = env->NewObject(constructorcls, constr, objcls->getClassObject(), //declaring class
          parametertypes, //parameterTypes
          checkedExceptions, //checkedExceptions
          info->access_flags, //modifiers
          objcls->findMethodIndex(name, sig), //slot
          objcls->getConstant(CTX(env), info->descriptor_index).obj, //signature
          NULL, //annotations
          NULL //parameterAnnotations
      );
    env->SetObjectArrayElement(ret, constructorcount, constrobj);
    ++constructorcount;
  }

  return ret;
}

jobjectArray JNIEXPORT Java_java_lang_Class_getDeclaredFields0(JNIEnv* env, jobject object, jboolean publicOnly){
	if (publicOnly){
    TR_USE(Java_Runtime);
		TR_BREAK("publicOnly not implemented");
	}
	VMContext* ctx = CTX(env);
	VMObject* obj = (VMObject*)object;
	VMClass* objcls = (VMClass*)obj;
	Java::ClassFile& cls = objcls->getClassDefinition();

	VMClass* fieldcls = getVM()->findClass(CTX(env), "java/lang/reflect/Field");
	VMObjectArray* arr = getVM()->createObjectArray(CTX(env), fieldcls, cls.fields_count);

	for (unsigned i = 0; i < cls.fields_count; ++i){
		Java::field_info* info = cls.fields[i];
		std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
		VMObject* arrobj = getVM()->createObject(CTX(env), fieldcls);
		CTX(env)->push(arrobj);
		CTX(env)->push(objcls->getClassObject());
		CTX(env)->push(objcls->getConstant(CTX(env), info->name_index).ui);
    //objcls->getinfo->descriptor_index
    VMObject* signature = objcls->getConstant(ctx, info->descriptor_index).obj;
    const char* sig = env->GetStringUTFChars(signature, NULL);
    std::string signat = sig;
    if (signat[0] == 'L'){
      signat.erase(0, 1);
      signat.erase(signat.size()-1);
    }
    VMClass* typeclass = getVM()->findClass(ctx, signat);
    env->ReleaseStringUTFChars(signature, sig);
		ctx->push(typeclass); //type class
		ctx->push(info->access_flags);
		ctx->push(objcls/*->findFieldIndex(name)*/);
		ctx->push(signature);
		ctx->push(0u);
		VMMethod* mthd = fieldcls->getMethod(fieldcls->findMethodIndex("<init>", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;IILjava/lang/String;[B)V"));
		mthd->execute(ctx);
		arr->put(arrobj, i);
	}
	
	return arr;
}

jint JNIEXPORT Java_java_lang_Class_getModifiers(JNIEnv* env, jclass clazz){
  VMClass* cls = (VMClass*)clazz;
  return cls->getClassDefinition().access_flags;
}

jstring JNIEXPORT Java_java_lang_Class_getName0(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime);
  VMClass* cls = (VMClass*)object;
  std::string name = getVM()->findClass(CTX(env), cls);
  if (name.empty()){
    TR_BREAK("Class not found");
    return NULL;
  }
  CGE::Utilities::replaceWith(name, '/', '.');
  jstring str = env->NewStringUTF(name.c_str());
  return str;
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
  else if (strcmp(namestr, "int") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "I");
  }
  else if (strcmp(namestr, "boolean") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "Z");
  }
	else{
    TR_USE(Java_Runtime);
		TR_BREAK("getPrimitiveClass not implemented for this type");
	}
	env->ReleaseStringUTFChars(name, namestr);
	return clazz;
}

jclass JNIEXPORT Java_java_lang_Class_getSuperclass(JNIEnv* env, jclass clazz){
  jclass super = env->GetSuperclass(clazz);
  return super;
}

jboolean JNIEXPORT Java_java_lang_Class_isInterface(JNIEnv* env, jobject object){
  VMClass* cls = (VMClass*)object;
  return cls->getClassDefinition().access_flags & ACC_INTERFACE ? JNI_TRUE : JNI_FALSE;
}

void JNIEXPORT Java_java_lang_ClassLoader_registerNatives(JNIEnv* env, jobject object){
	return;
}

void JNIEXPORT Java_java_lang_ClassLoader_00024NativeLibrary_load(JNIEnv* env, jobject object, jstring library){
  TR_USE(Java_Runtime);
  const char* libname = env->GetStringUTFChars(library, NULL);
  jlong handle = 0;
#ifdef WIN32
  HMODULE module = LoadLibrary(libname);
  handle = (jlong)module;
#endif
  //TODO
  env->ReleaseStringUTFChars(library, libname);
  jclass cls = env->FindClass("java/lang/ClassLoader$NativeLibrary");
  jfieldID handle_field = env->GetFieldID(cls, "handle", "J");
  env->SetLongField(object, handle_field, handle);
  return;
}


jlong JNIEXPORT Java_java_lang_Double_doubleToRawLongBits(JNIEnv* env, jobject object, jdouble value){
	return *((jlong*)(&value));
}

jdouble JNIEXPORT Java_java_lang_Double_longBitsToDouble(JNIEnv* env, jobject object, jlong value){
  return *((jdouble*)(&value));
}

jint JNIEXPORT Java_java_lang_Float_floatToRawIntBits(JNIEnv* env, jobject object, jfloat value){
	return *((jint*)(&value));
}

void JNIEXPORT Java_java_lang_Object_registerNatives(JNIEnv* env, jobject object){
	return;
}

jclass JNIEXPORT Java_java_lang_Object_getClass(JNIEnv* env, jobject object){
  jclass clazz = env->GetObjectClass(object);
  return clazz;
}

jint JNIEXPORT Java_java_lang_Object_hashCode(JNIEnv* env, jobject object){
	return (jint)(jlong)object;
}

jobject JNIEXPORT Java_java_lang_String_intern(JNIEnv* env, jobject object){
	const char* str = env->GetStringUTFChars((jstring)object, NULL);
	VMObject* ret = getVM()->internalizeString(str, (VMObject*)object);
	env->ReleaseStringUTFChars((jstring)object, str);
	return ret;
}

#ifdef UNIX
#define _getcwd getcwd
#endif

jobject JNIEXPORT Java_java_lang_System_initProperties(JNIEnv* env, jobject object, jobject properties){
  jclass cls = env->GetObjectClass(properties);
  jmethodID mthd = env->GetMethodID(cls, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");
  jstring key = env->NewStringUTF("file.separator");
  jstring value = env->NewStringUTF("\\");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("path.separator");
  value = env->NewStringUTF(";");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("user.dir");
  char userpath[1024];
  _getcwd(userpath, 1024);
  value = env->NewStringUTF(userpath);
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.library.path");
  value = env->NewStringUTF("D:\\Projects\\build_windows\\Compiler\\Debug");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("sun.boot.library.path");
  value = env->NewStringUTF("D:\\Projects\\build_windows\\Compiler\\Debug");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("file.encoding");
  value = env->NewStringUTF("UTF-8");
  env->CallObjectMethod(properties, mthd, key, value);
  return properties;
}

jstring JNIEXPORT Java_java_lang_System_mapLibraryName(JNIEnv* env, jobject object, jstring library){
  const char* str = env->GetStringUTFChars(library, NULL);
  std::string name = std::string(str)+".dll";
  jstring ret = env->NewStringUTF(name.c_str());
	env->ReleaseStringUTFChars(library, str);
  return ret;
}

void Java_java_lang_System_registerNatives(JNIEnv* env, jobject object){
	/*JNINativeMethod methods[3];
	methods[0].name = (char*)"currentTimeMillis";
	methods[0].signature = (char*)"()J";
	methods[0].fnPtr = (void*)Java_java_lang_System_currentTimeMillis;
	methods[1].name = (char*)"nanoTime";
	methods[1].signature = (char*)"()J";
	methods[1].fnPtr = (void*)Java_java_lang_System_nanoTime;
	methods[2].name = (char*)"arraycopy";
	methods[2].signature = (char*)"(Ljava/lang/Object;ILjava/lang/Object;II)V";
	methods[2].fnPtr = (void*)Java_java_lang_System_arraycopy;
	env->RegisterNatives(object, methods, 3);*/
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
	VMArrayBase* arrSrc = (VMArrayBase*)src;
  VMArrayBase* arrDest = (VMArrayBase*)dest;
  arrSrc->copyTo(srcPos, arrDest, destPos, length);
}

void JNIEXPORT Java_java_lang_System_setIn0(JNIEnv* env, jclass clazz, jobject in){
  jfieldID field = env->GetStaticFieldID(clazz, "in", "Ljava/io/InputStream;");
  env->SetStaticObjectField(clazz, field, in);
}

void JNIEXPORT Java_java_lang_Thread_registerNatives(JNIEnv* env, jobject object){
}

jobject JNIEXPORT Java_java_lang_Thread_currentThread(JNIEnv* env, jclass clazz){
	return CTX(env)->getThread();
}

jboolean JNIEXPORT Java_java_lang_Thread_isAlive(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime);
	TR_WARN("not implemented");
	return JNI_FALSE;
}

void JNIEXPORT Java_java_lang_Thread_setPriority0(JNIEnv* env, jobject object, jint priority){
  TR_USE(Java_Runtime);
	TR_WARN("setPriority0 not implemented");
}

void JNIEXPORT Java_java_lang_Thread_start0(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime);
	TR_WARN("not implemented");
}

jobject JNIEXPORT Java_java_security_AccessController_doPrivileged(JNIEnv* env, jobject object, jobject action){
	VMObject* obj = (VMObject*)action;
	VMMethod* mthd = obj->getObjMethod(obj->getClass()->findMethodIndex("run", "()Ljava/lang/Object;"));
	CTX(env)->push(obj);
	mthd->execute(CTX(env));
	obj = CTX(env)->pop().obj;
	return obj;
}

jobject JNIEXPORT Java_java_security_AccessController_getStackAccessControlContext(JNIEnv* env, jclass clazz){
	TR_USE(Java_Runtime);
  TR_WARN("getStackAccessControlContext not implemented");
  return NULL;
}

void JNIEXPORT Java_sun_misc_Unsafe_registerNatives(JNIEnv* env, jobject object){
	return;
}

jlong JNIEXPORT Java_sun_misc_Unsafe_allocateMemory(JNIEnv* env, jobject unsafe, jlong size){
  void* memory = malloc((size_t)size);
  return (jlong)memory;
}

jboolean JNIEXPORT Java_sun_misc_Unsafe_compareAndSwapInt(JNIEnv* env, jobject unsafe, jobject object, jlong fieldOffset, jint expected, jint update){
  //TODO make atomic
  VMObject* obj = (VMObject*)object;
  FieldData* fd = obj->getObjField((int)fieldOffset);
  if (fd->i == expected){
    //update
    fd->i = update;
    return JNI_TRUE;
  }
  return JNI_FALSE;
}

void JNIEXPORT Java_sun_misc_Unsafe_freeMemory(JNIEnv* env, jobject object, jlong address){
  free((void*)address);
}

jbyte JNIEXPORT Java_sun_misc_Unsafe_getByte(JNIEnv* env, jobject object, jlong address){
  jbyte* data = (jbyte*)address;
  return *data;
}

jlong JNIEXPORT Java_sun_misc_Unsafe_objectFieldOffset(JNIEnv* env, jobject object, jobject field){
	jclass fieldcls = env->GetObjectClass(field);
	jmethodID getDeclaringClass = env->GetMethodID(fieldcls, "getDeclaringClass", "()Ljava/lang/Class;");
	VMClass* cls = (VMClass*)env->CallObjectMethod(field, getDeclaringClass);
	jmethodID getName = env->GetMethodID(fieldcls, "getName", "()Ljava/lang/String;");
	jstring fieldname = (jstring)env->CallObjectMethod(field, getName);
	const char* name = env->GetStringUTFChars(fieldname, NULL);
	int fieldidx = cls->findFieldIndex(name);
	env->ReleaseStringUTFChars(fieldname, name);
	return fieldidx;
}

void JNIEXPORT Java_sun_misc_Unsafe_putLong(JNIEnv* env, jobject object, jlong address, jlong x){
  jlong* data = (jlong*)address;
  *data = x;
}

void JNIEXPORT Java_sun_misc_VM_initialize(JNIEnv* env, jobject object){
}

jobject JNIEXPORT Java_sun_reflect_NativeConstructorAccessorImpl_newInstance0
(JNIEnv* env, jobject object, jobject constructor, jobjectArray arguments){
  TR_USE(Java_Runtime);
  jclass constclass = env->GetObjectClass(constructor);
  jmethodID getDeclaringClass = env->GetMethodID(constclass, "getDeclaringClass", "()Ljava/lang/Class;");
  jclass instanceclass = env->CallObjectMethod(constructor, getDeclaringClass);
  jmethodID getSlot = env->GetMethodID(constclass, "getSlot", "()I");
  jint slot = env->CallIntMethod(constructor, getSlot);
  //int numargs = env->GetArrayLength(arguments);
  if (arguments != NULL)
    TR_BREAK("Constructor arguments need to be implemented");
  jobject instance = env->NewObject(instanceclass, (jmethodID)slot);
  return instance;
}

jobject JNIEXPORT Java_sun_reflect_Reflection_getCallerClass(JNIEnv* env, jobject object, jint realFramesToSkip){
	VMContext* ctx = CTX(env);
	VMMethod* mthd = ctx->getFrameMethod(realFramesToSkip);
	return (VMObject*)mthd->getClass();
}

jint JNIEXPORT Java_sun_reflect_Reflection_getClassAccessFlags(JNIEnv* env, jobject object, jclass clazz){
  VMClass* cls = (VMClass*)clazz;
  int access = cls->getClassDefinition().access_flags;
  return access;
}

}
