#include <wx/wx.h>

#include "Interpreter.h"
#include <system/winceMain.h>
#include "jni.h"

int main(int argc, char* argv[])
{
  wxInitialize();
  char* filename = "test/Test";
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
  
  jvm->DestroyJavaVM();
  wxUninitialize();
  return 0;
}

