#include "Runtime.h"
#include <io/Tracing.h>

#ifdef WIN32
#else
#include <sys/stat.h>
#endif

TR_CHANNEL(Java_Runtime_IO);

extern "C"{

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

void JNIEXPORT Java_java_io_FileInputStream_close0(JNIEnv* env, jobject object){
  TR_USE(Java_Runtime_IO);
  jclass fileinputstream = env->GetObjectClass(object);
  jfieldID fdfield = env->GetFieldID(fileinputstream, "fd", "Ljava/io/FileDescriptor;");
  jobject fd = env->GetObjectField(object, fdfield);

  jclass fileDescriptor = env->GetObjectClass(fd);
  jfieldID handlefield = env->GetFieldID(fileDescriptor, "handle", "J");
  FILE* f = (FILE*)env->GetLongField(fd, handlefield);
  int ret = fclose(f);
  if (ret != 0)
    TR_BREAK("Implement: Throw IOException");
  env->SetLongField(fd, handlefield, -1);
}

void JNIEXPORT Java_java_io_FileInputStream_initIDs(JNIEnv* env, jobject object){
  return;
}

void JNIEXPORT Java_java_io_FileInputStream_open(JNIEnv* env, jobject object, jstring filename){
  TR_USE(Java_Runtime_IO);
  const char* name = env->GetStringUTFChars(filename, NULL);
  FILE* f = fopen(name, "r");
  if (f == NULL)
    TR_BREAK("Implement me: Throw FileNotFoundException");
  env->ReleaseStringUTFChars(filename, name);
  jclass fileinputstream = env->GetObjectClass(object);
  jfieldID fdfield = env->GetFieldID(fileinputstream, "fd", "Ljava/io/FileDescriptor;");
  jobject fd = env->GetObjectField(object, fdfield);
  
  jclass fileDescriptor = env->GetObjectClass(fd);
  jfieldID handlefield = env->GetFieldID(fileDescriptor, "handle", "J");
  env->SetLongField(fd, handlefield, (jlong)f);
}

void JNIEXPORT Java_java_io_FileOutputStream_initIDs(JNIEnv* env, jobject object){
  return;
}

void JNIEXPORT Java_java_io_FileOutputStream_writeBytes(JNIEnv* env, jobject object, jbyteArray bytes, int off, int len){
  jclass fileoutputstream = env->GetObjectClass(object);
  jmethodID getFD = env->GetMethodID(fileoutputstream, "getFD", "()Ljava/io/FileDescriptor;");
  jobject filedescriptor = env->CallObjectMethod(object, getFD);
  jclass fdclass = env->GetObjectClass(filedescriptor);
  jfieldID handlefield = env->GetFieldID(fdclass, "handle", "J");
  jlong handle = env->GetLongField(filedescriptor, handlefield);
  FILE* file = (FILE*)handle;
  jbyte* buffer = env->GetByteArrayElements(bytes, NULL);
  fwrite(buffer, 1, len, file);
  env->ReleaseByteArrayElements(bytes, buffer, 0);
  return;
}

jobject JNIEXPORT Java_java_io_FileSystem_getFileSystem(JNIEnv* env, jobject object){
  //#ifdef WIN32
  jclass fs = env->FindClass("java/io/WinNTFileSystem");
  //#else
  //  jclass fs = env->FindClass("java/io/UnixFileSystem");
  //#endif
  jmethodID constr = env->GetMethodID(fs, "<init>", "()V");
  jobject ret = env->NewObject(fs, constr);
  return ret;
}

void JNIEXPORT Java_java_io_ObjectStreamClass_initNative(JNIEnv* env, jobject object){

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
#else
  struct stat st;
  int ret = stat(str, &st);
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
#else
  if (ret != 0)
    return jattribs;
  jattribs |= 0x01; //exists
  if (S_ISDIR(st.st_mode))
    jattribs |= 0x04; //directory
  else if (S_ISREG(st.st_mode))
    jattribs |= 0x02; //regular
  if (str[0] != '\0' && str[0] == '.')
    jattribs |= 0x08; //hidden
#endif
  return jattribs;
}

jlong JNIEXPORT Java_java_io_WinNTFileSystem_getLength(JNIEnv* env, jobject object, jobject file){
  jclass filecls = env->GetObjectClass(file);
  jmethodID getAbsPath = env->GetMethodID(filecls, "getAbsolutePath", "()Ljava/lang/String;");
  jstring path = env->CallObjectMethod(file, getAbsPath);
  const char* str = env->GetStringUTFChars(path, NULL);
  FILE* f = fopen(str, "r");
  fseek(f, 0, SEEK_END);
  jlong size = ftell(f);
  fclose(f);
  env->ReleaseStringUTFChars(path, str);
  return size;
}

void JNIEXPORT Java_java_io_WinNTFileSystem_initIDs(JNIEnv* env, jobject object){

}

}
