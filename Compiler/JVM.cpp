
#include "JVM.h"

#include <iostream>

#include "VMContext.h"
#include "VMclass.h"
#include "ClassLoader.h"
#include "Trace.h"
#include "VMMethod.h"
#include "VMArray.h"

#define PROC_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_MAP_MODE

static JVM* globalVM = NULL;

JVM* getVM(){
	return globalVM;
}

JVM::JVM(){
	globalVM = this;
  JNIInvokeInterface_::reserved0 = NULL;
  JNIInvokeInterface_::DestroyJavaVM = DestroyJavaVM;
	init();
}

JVM::~JVM(){
	for (std::list<VMObject*>::iterator iter = mCreatedObjects.begin(); iter != mCreatedObjects.end(); ++iter){
		delete *iter;
	}
	mCreatedObjects.clear();
	globalVM = NULL;
  for (std::list<VMContext*>::iterator iter = mThreads.begin(); iter != mThreads.end(); ++iter){
    delete *iter;
  }
  mThreads.clear();
	for (std::map<std::string,VMClass*>::iterator iter = mClassResolver.begin(); iter != mClassResolver.end(); ++iter){
		delete iter->second;
	}
	mClassResolver.clear();
}

void JVM::init(){
#ifndef UNDER_CE
  char* tmp = getenv("ProgramFiles");
#else
  char* tmp = NULL;
#endif
  std::string prefix;
  if (tmp)
    prefix = tmp;
	if (!mRuntimeClasses.openFile(prefix+"/Java/jre6/lib/rt.jar")){
    //hack for my crappy environment
    prefix[0] = 'E';
    if (!mRuntimeClasses.openFile(prefix+"/Java/jre6/lib/rt.jar")){
      TRACE_ABORT(TRACE_JAVA, "Java runtime classes not found");
    }
  }
	//TODO temporary hack to be able to load java.dll
	//mRuntime.open(prefix+"/Java/jre6/bin/client","jvm");
	//mRuntime.addSearchPath(prefix+"/Java/jre6/bin;"+prefix+"/Java/jre6/bin/client");
	//mRuntime.addSearchPath(prefix+"/Java/jre6/bin");
	if (!mRuntime.open("C:\\Projects\\engine_windows\\Compiler\\Debug","javaruntime")){
		TRACE_ABORT(TRACE_JAVA, "Java runtime not found");
	}
  Opcode::init();
}

VMClass* JVM::findClass(VMContext* ctx, std::string name){
  VMClass* entry = mClassResolver[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      //ClassData* clsdat = new ClassData();
      //clsdat->mLoader = NULL;
      //mClassData.push_back(clsdat);
      //entry = mClassData.size()-1;
      //mClassResolver[name] = entry;
      return NULL;
    }
    //Java::ClassFile* clfile = new Java::ClassFile();
		entry = new VMClass(name);
		mClassResolver[name] = entry;
		//init superclass first
		entry->getSuperclass(ctx);
		entry->print(std::cout);
		VMMethod* mthd = entry->findMethod("<clinit>", "()V");
		if (mthd){
			TRACE(TRACE_JAVA, TRACE_INFO, "Found class init method");
			mthd->execute(ctx);
		}
  }
  return entry;
}

CGE::MemReader JVM::getClassFile(const std::string& filename){
	return mRuntimeClasses.openEntry(filename);
}

nativeMethod JVM::findNativeMethod(const std::string& name){
	nativeMethod mthd = (nativeMethod)mRuntime.getFunction(name.c_str());
	return mthd;
}

VMArray* JVM::createArray(unsigned size){
	VMArray* arr = new VMArray(size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMObject* JVM::createObject(VMClass* cls){
	VMObject* obj = new VMObject(cls);
	mCreatedObjects.push_back(obj);
	return obj;
}
