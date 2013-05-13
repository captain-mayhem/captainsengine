
#include "Runtime.h"

#ifdef WIN32
#include <direct.h>
#else
#include <dlfcn.h>
#endif

#include <VMContext.h>
#include <VMMethod.h>
#include <VMclass.h>
#include <JVM.h>
#include <VMArray.h>
#include <Trace.h>
#include <system/utilities.h>
#include <VMLoader.h>

#include <iostream>
#include <signal.h>

TR_CHANNEL(Java_Runtime);

#define CTX(env) ((VMContext*)env->m_func)

extern "C" {

void JNIEXPORT Java_java_lang_Class_registerNatives(JNIEnv* env, jobject object){
	return;
}

jclass JNIEXPORT Java_java_lang_Class_forName0(JNIEnv* env, jclass clazz, jstring name, jboolean initialize, jobject classloader){
  TR_USE(Java_Runtime);
  const char* namestr = env->GetStringUTFChars(name, NULL);
  std::string str = namestr;
  CGE::Utilities::replaceWith(str, '.', '/');
  jclass retclass = NULL;
  if (initialize == JNI_TRUE){
    retclass = env->FindClass(str.c_str());
  }
  else{
    TR_BREAK("Implement me");
    //retclass = getVM()->defineClass(CTX(env), str);
  }
  env->ReleaseStringUTFChars(name, namestr);
  return retclass;
}

jboolean JNIEXPORT Java_java_lang_Class_desiredAssertionStatus0(JNIEnv* env, jobject object, jobject cls){
	return JNI_FALSE;
}

jobject JNIEXPORT Java_java_lang_Class_getClassLoader0(JNIEnv* env, jobject object){
  VMClass* cls = (VMClass*)object;
	return cls->getLoader()->getLoaderObject();
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

	VMClass* fieldcls = getVM()->getLoader(NULL)->find(CTX(env), "java/lang/reflect/Field");
	VMObjectArray* arr = getVM()->createObjectArray(CTX(env), fieldcls, cls.fields_count);

	for (unsigned i = 0; i < cls.fields_count; ++i){
		Java::field_info* info = cls.fields[i];
		std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
		VMObject* arrobj = getVM()->createObject(CTX(env), fieldcls);
		CTX(env)->push(arrobj);
		CTX(env)->push(objcls->getClassObject());
    VMObject* nameobj = objcls->getConstant(CTX(env), info->name_index).obj;
    nameobj = getVM()->internalizeString(name, nameobj);
		CTX(env)->push(nameobj);
    //objcls->getinfo->descriptor_index
    VMObject* signature = objcls->getConstant(ctx, info->descriptor_index).obj;
    const char* sig = env->GetStringUTFChars(signature, NULL);
    std::string signat = sig;
    if (signat[0] == 'L'){
      signat.erase(0, 1);
      signat.erase(signat.size()-1);
    }
    VMClass* typeclass = objcls->getLoader()->find(ctx, signat);
    env->ReleaseStringUTFChars(signature, sig);
		ctx->push(typeclass); //type class
		ctx->push(info->access_flags);
		ctx->push(objcls->findFieldIndex(name));
		ctx->push(signature);
		ctx->push(0u);
		VMMethod* mthd = fieldcls->getMethod(fieldcls->findMethodIndex("<init>", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;IILjava/lang/String;[B)V"));
		mthd->execute(ctx, -2);
		arr->put(arrobj, i);
	}
	
	return arr;
}

jobjectArray JNIEXPORT Java_java_lang_Class_getDeclaredMethods0(JNIEnv* env, jobject object, jboolean publicOnly){
  TR_USE(Java_Runtime);
  if (publicOnly){
    TR_BREAK("publicOnly not implemented");
  }
  //VMContext* ctx = CTX(env);
  //VMObject* obj = (VMObject*)object;
  VMClass* objcls = (VMClass*)object;
  Java::ClassFile& cls = objcls->getClassDefinition();

  int methodcount = 0;
  for (int i = 0; i < cls.methods_count; ++i){
    Java::method_info* info = cls.methods[i];
    std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
    if (name != "<init>")
      ++methodcount;
  }

  jclass methodcls = env->FindClass("java/lang/reflect/Method");
  jmethodID constr = env->GetMethodID(methodcls, "<init>", "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Class;Ljava/lang/Class;[Ljava/lang/Class;IILjava/lang/String;[B[B[B)V");
  jobjectArray ret = env->NewObjectArray(methodcount, methodcls, NULL);

  methodcount = 0;
  for (int i = 0; i < cls.methods_count; ++i){
    Java::method_info* info = cls.methods[i];
    std::string name = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->name_index-1]))->bytes;
    if (name == "<init>")
      continue;
    std::string  sig = ((Java::CONSTANT_Utf8_info*)(cls.constant_pool[info->descriptor_index-1]))->bytes;
    unsigned id = objcls->findMethodIndex(name, sig);
    VMMethod* mthd = objcls->getMethod(id);
    jclass theobjectclass = env->FindClass("java/lang/Class");
    jobjectArray parametertypes = env->NewObjectArray(mthd->getNumParameters(), theobjectclass, NULL);

    for (unsigned j = 0; j < mthd->getNumParameters(); ++j){
      VMClass* cls = mthd->getParameterClass(CTX(env), j);
      env->SetObjectArrayElement(parametertypes, j, cls);
    }
    jobjectArray checkedExceptions = env->NewObjectArray(0, theobjectclass, NULL);
    VMObject* methodname = objcls->getConstant(CTX(env), info->name_index).obj;
    methodname = getVM()->internalizeString(name, methodname);
    jobject methodobj = env->NewObject(methodcls, constr, objcls->getClassObject(), //declaring class
      methodname, //name
      parametertypes, //parameterTypes
      mthd->getParameterClass(CTX(env), -1), //returnType
      checkedExceptions, //checkedExceptions
      info->access_flags, //modifiers
      objcls->findMethodIndex(name, sig), //slot
      objcls->getConstant(CTX(env), info->descriptor_index).obj, //signature
      NULL, //annotations
      NULL, //parameterAnnotations
      NULL //annotationDefault
      );
    env->SetObjectArrayElement(ret, methodcount, methodobj);
    ++methodcount;
  }

  return ret;
}

jint JNIEXPORT Java_java_lang_Class_getModifiers(JNIEnv* env, jclass clazz){
  VMClass* cls = (VMClass*)clazz;
  return cls->getClassDefinition().access_flags;
}

jstring JNIEXPORT Java_java_lang_Class_getName0(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime);
  VMClass* cls = (VMClass*)object;
  std::string name = cls->getName();
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
  else if (strcmp(namestr, "void") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "V");
  }
  else if (strcmp(namestr, "byte") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "B");
  }
  else if (strcmp(namestr, "long") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "J");
  }
  else if (strcmp(namestr, "char") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "C");
  }
  else if (strcmp(namestr, "short") == 0){
    clazz = getVM()->getPrimitiveClass(CTX(env), "S");
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

jboolean JNIEXPORT Java_java_lang_Class_isArray(JNIEnv* env, jclass cls){
  return JNI_FALSE; //Arrays are handled separately
}

jboolean JNIEXPORT Java_java_lang_Class_isInterface(JNIEnv* env, jobject object){
  VMClass* cls = (VMClass*)object;
  return cls->getClassDefinition().access_flags & ACC_INTERFACE ? JNI_TRUE : JNI_FALSE;
}

jboolean JNIEXPORT Java_java_lang_Class_isPrimitive(JNIEnv* env, jclass cls){
  VMClass* clazz = (VMClass*)cls;
  if (clazz->getName().size() == 1)
    return JNI_TRUE;
  return JNI_FALSE;
}

void JNIEXPORT Java_java_lang_ClassLoader_registerNatives(JNIEnv* env, jobject object){
	return;
}

void JNIEXPORT Java_java_lang_ClassLoader_00024NativeLibrary_load(JNIEnv* env, jobject object, jstring library){
  TR_USE(Java_Runtime);
  jclass nl = env->GetObjectClass(object);
  //jmethodID getFromClass = env->GetMethodID(nl, "getFromClass", "()Ljava/lang/Class;");
  //jclass fromClass = env->CallStaticObjectMethod(nl, getFromClass);
  jfieldID fromClassField = env->GetStaticFieldID(nl, "fromClass", "Ljava/lang/Class;");
  jclass fromClass = env->GetObjectField(object, fromClassField);
  VMLoader* ldr = ((VMClass*)fromClass)->getLoader();
  const char* libname = env->GetStringUTFChars(library, NULL);
  jlong handle = ldr->addLibrary(libname);
  env->ReleaseStringUTFChars(library, libname);
  jfieldID handle_field = env->GetFieldID(nl, "handle", "J");
  env->SetLongField(object, handle_field, handle);
  return;
}

jclass JNIEXPORT Java_java_lang_ClassLoader_defineClass1(JNIEnv* env, jobject object, jstring name, jbyteArray b, int off, int len,
                                                         jobject pd, jstring source, jboolean verify){
  TR_USE(Java_Runtime);
  jbyte* clsdata = env->GetByteArrayElements(b, NULL);
  CGE::MemReader rdr(clsdata+off, len);
  VMObject* ldrobj = (VMObject*)object;
  VMLoader* ldr = getVM()->getLoader(ldrobj);
  const char* namestr = env->GetStringUTFChars(name, NULL);
  VMClass* cls = ldr->load(CTX(env), namestr, rdr);
  if (cls == NULL)
    TR_BREAK("Error defining class");
  env->ReleaseStringUTFChars(name, namestr);
  env->ReleaseByteArrayElements(b, clsdata, 0);
  return cls;
}

jobject JNIEXPORT Java_java_lang_ClassLoader_findBootstrapClass(JNIEnv* env, jobject object, jstring name){
  const char* clsname = env->GetStringUTFChars(name, NULL);
  std::string realname = clsname;
  CGE::Utilities::replaceWith(realname, '.', '/');
  VMClass* cls = getVM()->getLoader(NULL)->find(CTX(env), realname);
  env->ReleaseStringUTFChars(name, clsname);
  return cls;
}

jobject JNIEXPORT Java_java_lang_ClassLoader_findLoadedClass0(JNIEnv* env, jobject object, jstring name){
  VMLoader* ldr = getVM()->getLoader((VMObject*)object);
  const char* clsname = env->GetStringUTFChars(name, NULL);
  VMClass* cls = ldr->get(clsname);
  env->ReleaseStringUTFChars(name, clsname);
  return cls;
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

void JNIEXPORT Java_java_lang_Object_wait(JNIEnv* env, jobject object, jlong timeout){
  TR_USE(Java_Runtime);
  VMObject* obj = (VMObject*)object;
  if (timeout == 0)
    obj->wait();
  else
    TR_BREAK("Implement me");
}

jlong JNIEXPORT Java_java_lang_Runtime_freeMemory(JNIEnv* env, jobject object){
	//TODO
	return 1024*1024*1000;
}

jobject JNIEXPORT Java_java_lang_String_intern(JNIEnv* env, jobject object){
	const char* str = env->GetStringUTFChars((jstring)object, NULL);
	VMObject* ret = getVM()->internalizeString(str, (VMObject*)object);
	env->ReleaseStringUTFChars((jstring)object, str);
	return ret;
}

jint JNIEXPORT Java_java_lang_System_identityHashCode(JNIEnv* env, jclass unused, jobject object){
	return (jint)(jlong)object;
}

#ifdef UNIX
#define _getcwd getcwd
#endif

jobject JNIEXPORT Java_java_lang_System_initProperties(JNIEnv* env, jobject object, jobject properties){
  jclass cls = env->GetObjectClass(properties);
  jmethodID mthd = env->GetMethodID(cls, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");
  VMArgs* args = getVM()->getArguments();

  /*jstring key = env->NewStringUTF("java.version");
  jstring value = env->NewStringUTF("1.6.0");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.vendor");
  value = env->NewStringUTF("Captain");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.vendor.url");
  value = env->NewStringUTF("www.captain-online.de");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.home");
  const char* java_home = getenv("JAVA_HOME");
  value = env->NewStringUTF(java_home);
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.vm.specification.version");
  value = env->NewStringUTF("1.6");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.vm.specification.vendor");
  value = env->NewStringUTF("Captain");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("java.vm.specification.name");
  value = env->NewStringUTF("Virtual Machine Specifications");
  env->CallObjectMethod(properties, mthd, key, value);*/

  jstring key = env->NewStringUTF("file.separator");
#ifdef WIN32
  jstring value = env->NewStringUTF("\\");
#else
  jstring value = env->NewStringUTF("/");
#endif
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("path.separator");
#ifdef WIN32
  value = env->NewStringUTF(";");
#else
  value = env->NewStringUTF(":");
#endif
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("line.separator");
  value = env->NewStringUTF("\n");
  env->CallObjectMethod(properties, mthd, key, value);
  key = env->NewStringUTF("user.dir");
  char userpath[1024];
#ifdef WIN32
  _getcwd(userpath, 1024);
#else
  userpath[0] = '\0';
#endif
  value = env->NewStringUTF(userpath);
  env->CallObjectMethod(properties, mthd, key, value);
#ifdef WIN32
  char filename[1024];
  GetModuleFileName(GetModuleHandle(NULL), filename, 1024);
  const char* tmp = strrchr(filename, '\\');
  filename[tmp-filename] = '\0';
#else
  char filename[1024];
  readlink("/proc/self/exe", filename, 1024);
  *strrchr(filename, '/') = '\0';
#endif
  /*key = env->NewStringUTF("java.library.path");
  value = env->NewStringUTF(filename);
  env->CallObjectMethod(properties, mthd, key, value);*/
  key = env->NewStringUTF("sun.boot.library.path");
  value = env->NewStringUTF(filename);
  env->CallObjectMethod(properties, mthd, key, value);
  
  key = env->NewStringUTF("file.encoding");
  value = env->NewStringUTF("UTF-8");
  env->CallObjectMethod(properties, mthd, key, value);
  
  /*key = env->NewStringUTF("java.protocol.handler.pkgs");
  value = env->NewStringUTF("");
  env->CallObjectMethod(properties, mthd, key, value);*/
  
  key = env->NewStringUTF("java.class.path");
  value = env->NewStringUTF(args->classpath);
  env->CallObjectMethod(properties, mthd, key, value);

  //init user properties
  char** props = args->properties;
  while (props != NULL && *props != NULL){
    char* prop = *props;
    char* val = strchr(prop, '=');
    if (val != NULL){
      *val = '\0';
      ++val;
      value = env->NewStringUTF(val);
    }
    else
      value = env->NewStringUTF("true");
    key = env->NewStringUTF(prop);
    env->CallObjectMethod(properties, mthd, key, value);
    ++props;
  }
  return properties;
}

jstring JNIEXPORT Java_java_lang_System_mapLibraryName(JNIEnv* env, jobject object, jstring library){
  const char* str = env->GetStringUTFChars(library, NULL);
#ifdef WIN32
  std::string name = std::string(str)+".dll";
#else
  std::string name = "lib"+std::string(str)+".so";
#endif
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
#else
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv.tv_sec*1000+tv.tv_nsec/1000000;
#endif
}

jlong JNIEXPORT Java_java_lang_System_nanoTime(JNIEnv* env, jobject object){
#ifdef WIN32
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return (jlong)(counter.QuadPart*1000000000/(double)frequency.QuadPart);
#else
	struct timespec tv;
        clock_gettime(CLOCK_REALTIME, &tv);
        return tv.tv_sec*1000+tv.tv_nsec/1000000;
#endif
}

void JNIEXPORT Java_java_lang_System_arraycopy(JNIEnv* env, jobject object, jobject src, int srcPos, jobject dest, int destPos, int length){
	VMArrayBase* arrSrc = (VMArrayBase*)src;
  VMArrayBase* arrDest = (VMArrayBase*)dest;
  arrSrc->copyTo(srcPos, arrDest, destPos, length);
}

void JNIEXPORT Java_java_lang_System_setErr0(JNIEnv* env, jclass clazz, jobject err){
  jfieldID field = env->GetStaticFieldID(clazz, "err", "Ljava/io/PrintStream;");
  env->SetStaticObjectField(clazz, field, err);
}

void JNIEXPORT Java_java_lang_System_setIn0(JNIEnv* env, jclass clazz, jobject in){
  jfieldID field = env->GetStaticFieldID(clazz, "in", "Ljava/io/InputStream;");
  env->SetStaticObjectField(clazz, field, in);
}

void JNIEXPORT Java_java_lang_System_setOut0(JNIEnv* env, jclass clazz, jobject out){
  jfieldID field = env->GetStaticFieldID(clazz, "out", "Ljava/io/PrintStream;");
  env->SetStaticObjectField(clazz, field, out);
}

void JNIEXPORT Java_java_lang_Thread_registerNatives(JNIEnv* env, jobject object){
}

jobject JNIEXPORT Java_java_lang_Thread_currentThread(JNIEnv* env, jclass clazz){
	return CTX(env)->getThread();
}

jboolean JNIEXPORT Java_java_lang_Thread_isAlive(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime);
  jclass thread = env->GetObjectClass(object);
  jfieldID eetop = env->GetFieldID(thread, "eetop", "J");
  jlong theThread = env->GetLongField(object, eetop);
  if (theThread == 0)
    return JNI_FALSE;
  TR_BREAK("Implement me");
	return JNI_FALSE;
}

jboolean JNIEXPORT Java_java_lang_Thread_isInterrupted(JNIEnv* env, jobject object, jboolean clearInterrupted){
  return JNI_FALSE;
}

void JNIEXPORT Java_java_lang_Thread_setPriority0(JNIEnv* env, jobject object, jint priority){
  TR_USE(Java_Runtime);
  jclass thread = env->GetObjectClass(object);
  jfieldID eetop = env->GetFieldID(thread, "eetop", "J");
  jlong theThread = env->GetLongField(object, eetop);
  if (theThread == 0)
    return;
	TR_BREAK("setPriority0 not implemented");
}

void threadRoutine(void* parameter){
  VMObject* thrd = (VMObject*)parameter;
  VMContext* ctx = getVM()->createContext(thrd);
  JNIEnv* env = ctx->getJNIEnv();

  jclass cls = env->GetObjectClass(thrd);
  jmethodID run = env->GetMethodID(cls, "run", "()V");
  env->CallVoidMethod(thrd, run);

  jthrowable exception = env->ExceptionOccurred();
  if (exception != NULL){
    printf("Uncaught exception occurred!\n");
    env->ExceptionDescribe();
  }

  getVM()->destroyContext(ctx);
}

static CGE::Thread::Priority javaToCGEThreadPrio(int prio){
  switch (prio){
    case 5:
      return CGE::Thread::NORMAL;
    case 4:
    case 3:
      return CGE::Thread::LOW;
    case 6:
    case 7:
      return CGE::Thread::HIGH;
    case 2:
      return CGE::Thread::VERY_LOW;
    case 1:
      return CGE::Thread::EXTREMELY_LOW;
    case 8:
    case 9:
      return CGE::Thread::VERY_HIGH;
    case 10:
      return CGE::Thread::EXTREMELY_HIGH;
    default:
      return CGE::Thread::NORMAL;
  }
}

void JNIEXPORT Java_java_lang_Thread_start0(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime);
  jclass thread = env->GetObjectClass(object);
  jfieldID eetop = env->GetFieldID(thread, "eetop", "J");
  CGE::Thread* newthrd = new CGE::Thread;
  jfieldID prio = env->GetFieldID(thread, "priority", "I");
  int priority = env->GetIntField(object, prio);
  newthrd->create(threadRoutine, object);
  newthrd->setPriority(javaToCGEThreadPrio(priority));
  env->SetLongField(object, eetop, (jlong)newthrd);
}

jint JNIEXPORT Java_java_lang_Throwable_getStackTraceDepth(JNIEnv* env, jobject object){
  jclass throwable = env->GetObjectClass(object);
  jfieldID backtrace = env->GetFieldID(throwable, "backtrace", "Ljava/lang/Object;");
  jobjectArray stack = env->GetObjectField(object, backtrace);
  jsize size = env->GetArrayLength(stack)/2;
  return (jint)size;
}

jobject JNIEXPORT Java_java_lang_Throwable_getStackTraceElement(JNIEnv* env, jobject object, jint index){
  jclass throwable = env->GetObjectClass(object);
  jfieldID backtrace = env->GetFieldID(throwable, "backtrace", "Ljava/lang/Object;");
  jobjectArray stack = env->GetObjectField(object, backtrace);
  VMMethod* mthd = (VMMethod*) env->GetObjectArrayElement(stack, 2*index);
  int ip = (int)(intptr_t) env->GetObjectArrayElement(stack, 2*index+1);
  
  jclass StackTraceElement = env->FindClass("java/lang/StackTraceElement");
  jmethodID steconstr = env->GetMethodID(StackTraceElement, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
  jstring clsname = env->NewStringUTF(mthd->getClass()->getName().c_str());
  jstring methodname = env->NewStringUTF(mthd->getName().c_str());
  jstring filename = NULL;
  std::string sourceFile = mthd->getClass()->getSourceFileName();
  if (!sourceFile.empty())
    filename = env->NewStringUTF(sourceFile.c_str());
  unsigned linenumber = mthd->getClass()->getLineNumber(ip, mthd->getMethodIndex());
  jobject elem = env->NewObject(StackTraceElement, steconstr, clsname, methodname, filename, linenumber);
  return elem;
}

jobject JNIEXPORT Java_java_lang_Throwable_fillInStackTrace(JNIEnv* env, jobject object){
  jclass throwable = env->GetObjectClass(object);
  jfieldID backtrace = env->GetFieldID(throwable, "backtrace", "Ljava/lang/Object;");
  VMContext* ctx = CTX(env);
  VMMethod* mthd = NULL;
  std::vector<VMMethod*> stack;
  std::vector<unsigned> ips;
  unsigned lastIp = -1;
  VMContext::StackIterator iter = ctx->getTopFrame();
  bool doNext = false;
  do{
    if (doNext)
      iter.next();
    else
      doNext = true;
    mthd = iter.getMethod();
    if (mthd == NULL)
      break;
    unsigned returnaddr = iter.getReturnIP();
    jclass realthrowable = env->FindClass("java/lang/Throwable");
    if (!env->IsAssignableFrom(mthd->getClass(), realthrowable)){ //hide the stack trace caused by exception itself
      stack.push_back(mthd);
      ips.push_back(lastIp);
    }
    lastIp = returnaddr;
  } while(iter.hasNext());
  jclass objcls = env->FindClass("java/lang/Object");
  jobjectArray trace = env->NewObjectArray((jsize)stack.size()*2, objcls, NULL);
  for (unsigned i = 0; i < stack.size(); ++i){
    env->SetObjectArrayElement(trace, 2*i, stack[i]);
    env->SetObjectArrayElement(trace, 2*i+1, (jobject)ips[i]);
  }
  env->SetObjectField(object, backtrace, trace);
  //null stackTrace
  jfieldID sttrace = env->GetFieldID(throwable, "stackTrace", "[Ljava/lang/StackTraceElement;");
  env->SetObjectField(object, sttrace, NULL);
  return object;
}

jobject JNIEXPORT Java_java_security_AccessController_doPrivileged(JNIEnv* env, jobject object, jobject action){
	VMObject* obj = (VMObject*)action;
	VMMethod* mthd = obj->getObjMethod(obj->getClass()->findMethodIndex("run", "()Ljava/lang/Object;"));
	CTX(env)->push(obj);
	mthd->execute(CTX(env), -2);
	obj = CTX(env)->pop().obj;
	return obj;
}

jobject JNIEXPORT Java_java_security_AccessController_getStackAccessControlContext(JNIEnv* env, jclass clazz){
	TR_USE(Java_Runtime);
  TR_WARN("getStackAccessControlContext not implemented");
  return NULL;
}

jlong JNIEXPORT Java_sun_io_Win32ErrorMode_setErrorMode(JNIEnv* env, jclass clazz, jlong mode){
#ifdef WIN32
  return SetErrorMode((UINT)mode);
#endif
}

jint JNIEXPORT Java_sun_misc_Signal_findSignal(JNIEnv* env, jobject object, jstring signal){
  TR_USE(Java_Runtime);
  jint ret = -1;
  const char* sig = env->GetStringUTFChars(signal, NULL);
  if (strcmp(sig, "INT") == 0){
    ret = SIGINT;
  }
  else if (strcmp(sig, "TERM") == 0){
    ret = SIGTERM;
  }
  else
    TR_BREAK("Unknown signal %s", sig);
  env->ReleaseStringUTFChars(signal, sig);
  return ret;
}

typedef void (*pfnSignalHandler)(int);
static JNIEnv* mainThreadEnv = NULL;

void callbackSignalHandler(int signal){
  TR_USE(Java_Runtime);
  //call signal dispatch
  jclass cls = mainThreadEnv->FindClass("sun/misc/Signal");
  jmethodID dispatch = mainThreadEnv->GetStaticMethodID(cls, "dispatch", "(I)V");
  mainThreadEnv->CallStaticVoidMethod(cls, dispatch, signal);
  TR_BREAK("Should callback signal dispatch - how to get a JNIEnv?")
}

jlong JNIEXPORT Java_sun_misc_Signal_handle0(JNIEnv* env, jobject object, jint sig, jlong nativeH){
  TR_USE(Java_Runtime);
  if (mainThreadEnv == NULL)
    mainThreadEnv = env;
  pfnSignalHandler prev;
  if (nativeH == 2) //call signal dispatch
    prev = signal(sig, callbackSignalHandler);
  else
    TR_BREAK("Unknown native handle");
  return (jlong)prev;
}

void JNIEXPORT Java_sun_misc_Unsafe_registerNatives(JNIEnv* env, jobject object){
	return;
}

jint JNIEXPORT Java_sun_misc_Unsafe_addressSize(JNIEnv* env, jobject unsafe, jclass arrayClass){
	return sizeof(void*);
}

jlong JNIEXPORT Java_sun_misc_Unsafe_allocateMemory(JNIEnv* env, jobject unsafe, jlong size){
  void* memory = malloc((size_t)size);
  return (jlong)memory;
}

jint JNIEXPORT Java_sun_misc_Unsafe_arrayBaseOffset(JNIEnv* env, jobject unsafe, jclass arrayClass){
	return 0;
}

jint JNIEXPORT Java_sun_misc_Unsafe_arrayIndexScale(JNIEnv* env, jobject unsafe, jclass arrayClass){
	return 0;
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
