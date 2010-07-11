#include <iostream>

#include <wx/wx.h>

//#include "Interpreter.h"
#include <system/winceMain.h>

#include "jni.h"

int main(int argc, char* argv[])
{
  wxInitialize();
  char* filename = "test/Test";
	//char* filename = "java/lang/Object";
  if (argc > 1){
    filename = argv[1];
  }
  
  //Interpreter interp;
  //interp.execute(filename);
	
  JavaVM* jvm;
  JNIEnv* env;
  JDK1_1InitArgs vm_args;

  vm_args.version = 0x00010001;
  JNI_GetDefaultJavaVMInitArgs(&vm_args);
  //vm_args.classpath = "";
  JNI_CreateJavaVM(&jvm, &env, &vm_args);
  
  jclass cls = env->FindClass(filename);
	jmethodID mainfunc = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
	if (mainfunc == NULL){
		std::cerr << "No main function found" << std::endl;
		jvm->DestroyJavaVM();
		wxUninitialize();
		return -1;
	}

	jclass string = env->FindClass("java/lang/String");
	jobjectArray args = env->NewObjectArray(1, string, NULL);
	jstring test = env->NewStringUTF("teststring");
	env->CallStaticVoidMethod(cls, mainfunc, args);
  
  jvm->DestroyJavaVM();
  wxUninitialize();
  return 0;
}

