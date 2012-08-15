#include <iostream>

#include <system/engine.h>
#include <system/allocation.h>
#include <system/winceMain.h>
#include "Trace.h"


#include "jni.h"

enum ArgType{
  ArgNone,
  ArgClass,
  ArgCp,
  ArgCpPath,
  ArgProp,
  ArgPropProperty,
  ArgProperty,
};

ArgType parseArgument(ArgType prevArg, const char* arg){
  if (prevArg == ArgCp)
    return ArgCpPath;
  if (prevArg == ArgProp)
    return ArgPropProperty;
  if (strcmp(arg, "-D") == 0)
    return ArgProp;
  if (strncmp(arg, "-D", 2) == 0)
    return ArgProperty;
  if (strcmp(arg, "-cp") == 0 || strcmp(arg, "-classpath") == 0)
    return ArgCp;
  return ArgClass;
}



int main(int argc, char* argv[])
{
  /*CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
	CGE::Engine::instance()->init();*/
  std::vector<char*> properties;

  JDK1_1InitArgs vm_args;
  vm_args.version = 0x00010001;
  vm_args.classpath = ".";
  const char* filename = "Test";
	//char* filename = "java/lang/Object";
  ArgType prev = ArgNone;
  for (int i = 0; i < argc; ++i){
    ArgType type = parseArgument(prev, argv[i]);
    switch(type){
      case ArgClass:
        filename = argv[i];
        break;
      case ArgCpPath:
        vm_args.classpath = argv[i];
        break;
      case ArgProperty:
        properties.push_back(argv[i]+2);
        break;
      case ArgPropProperty:
        properties.push_back(argv[i]);
        break;
    }
    prev = type;
  }

  properties.push_back(NULL);
  vm_args.properties = properties.empty() ? NULL : &properties[0];
  
  //Interpreter interp;
  //interp.execute(filename);
	
  JavaVM* jvm;
  JNIEnv* env;

  JNI_GetDefaultJavaVMInitArgs(&vm_args);
  //vm_args.classpath = "";
  JNI_CreateJavaVM(&jvm, &env, &vm_args);
  
  jclass cls = env->FindClass(filename);
	jmethodID mainfunc = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
	if (mainfunc == NULL){
		std::cerr << "No main function found" << std::endl;
		jvm->DestroyJavaVM();
		return -1;
	}

	jclass string = env->FindClass("java/lang/String");
	jobjectArray args = env->NewObjectArray(1, string, NULL);
	//jstring test = env->NewStringUTF("teststring");
	env->CallStaticVoidMethod(cls, mainfunc, args);

  jthrowable exception = env->ExceptionOccurred();
  if (exception != NULL){
    printf("Uncaught exception occurred!\n");
    env->ExceptionDescribe();
  }
  
  jvm->DestroyJavaVM();
	//CGE::Engine::instance()->shutdown();
  return 0;
}

