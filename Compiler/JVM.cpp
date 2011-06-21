
#include "JVM.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

#include "VMContext.h"
#include "VMclass.h"
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
	for (std::map<std::string,VMClass*>::reverse_iterator iter = mClassResolver.rbegin(); iter != mClassResolver.rend(); ++iter){
		delete iter->second;
	}
	mClassResolver.clear();
}

void JVM::init(){
	TRACE_ENABLE(TRACE_JAVA);
	TRACE_ENABLE(VM_METHODS);
  std::string prefix;
#ifndef UNDER_CE
  char* tmp = getenv("JAVA_HOME");
  if (tmp != NULL){
    prefix = tmp;
  }
  else{
    tmp = getenv("ProgramFiles");
    if (tmp != NULL){
      prefix = tmp;
      prefix += "/Java/jre6";
    }
  }
#endif
	if (!mRuntimeClasses.openFile(prefix+"/lib/rt.jar")){
    //hack for my crappy environment
    prefix[0] = 'E';
    if (!mRuntimeClasses.openFile(prefix+"/lib/rt.jar")){
      TRACE_ABORT(TRACE_JAVA, "Java runtime classes not found");
    }
  }
	//TODO temporary hack to be able to load java.dll
	//mRuntime.open(prefix+"/Java/jre6/bin/client","jvm");
	//mRuntime.addSearchPath(prefix+"/Java/jre6/bin;"+prefix+"/Java/jre6/bin/client");
	//mRuntime.addSearchPath(prefix+"/Java/jre6/bin");
	if (!mRuntime.open("D:\\Projects\\build_windows\\Compiler\\Debug","javaruntime")){
		TRACE_ABORT(TRACE_JAVA, "Java runtime not found");
	}
  Opcode::init();
}

VMClass* JVM::findClass(VMContext* ctx, std::string name){
	std::map<std::string,VMClass*>::iterator iter = mUninitializedClasses.find(name);
	if (iter != mUninitializedClasses.end()){
		VMClass* cls = iter->second;
		mUninitializedClasses.erase(iter);
		mClassResolver[name] = cls;
		//delayed class init
		unsigned idx = cls->findMethodIndex("<clinit>", "()V");
		VMMethod* mthd = cls->getMethod(idx);
		if (mthd){
			TRACE(TRACE_JAVA, TRACE_INFO, "Delayed execution of class init method");
			mthd->execute(ctx);
		}
		return cls;
	}
  VMClass* entry = mClassResolver[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      entry = new VMArrayClass();
			mClassResolver[name] = entry;
      return entry;
    }
    //Java::ClassFile* clfile = new Java::ClassFile();
		entry = new VMClass(name);

		mClassResolver[name] = entry;

		//init superclass first
		entry->getSuperclass(ctx);
		
		//entry->print(std::cout);
		
		entry->initFields(ctx);

		VMClass* cls = findClass(ctx, "java/lang/Class");
		VMMethod* clsmthd = cls->getMethod(cls->findMethodIndex("<init>", "()V"));
		entry->init(ctx, cls);
		ctx->push((VMObject*)cls);
		clsmthd->execute(ctx);

		unsigned idx = entry->findMethodIndex("<clinit>", "()V");
		VMMethod* mthd = entry->getMethod(idx);
		if (mthd){
			TRACE(TRACE_JAVA, TRACE_INFO, "Found class init method");
			mthd->execute(ctx);
		}
  }
  return entry;
}

VMClass* JVM::defineClass(VMContext* ctx, const std::string& name){
	std::map<std::string,VMClass*>::iterator iter = mClassResolver.find(name);
	if (iter != mClassResolver.end())
		return iter->second;
	VMClass* entry = mUninitializedClasses[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      entry = new VMArrayClass();
			mClassResolver[name] = entry;
      return entry;
    }
    //Java::ClassFile* clfile = new Java::ClassFile();
		entry = new VMClass(name);

		mUninitializedClasses[name] = entry;

		//init superclass first
		entry->getSuperclass(ctx);
		
		//entry->print(std::cout);
		
		entry->initFields(ctx);

		VMClass* cls = findClass(ctx, "java/lang/Class");
		VMMethod* clsmthd = cls->getMethod(cls->findMethodIndex("<init>", "()V"));
		entry->init(ctx, cls);
		ctx->push((VMObject*)cls);
		clsmthd->execute(ctx);
  }
  return entry;
}

VMClass* JVM::getPrimitiveClass(VMContext* ctx, std::string name){
	 VMClass* entry = mClassResolver[name];
  if (entry == 0){
		entry = new VMClass();
		
		mClassResolver[name] = entry;
		//entry->print(std::cout);
		
		//entry->initFields(ctx);

		VMClass* cls = findClass(ctx, "java/lang/Class");
		VMMethod* clsmthd = cls->getMethod(cls->findMethodIndex("<init>", "()V"));
		entry->init(ctx, cls);
		ctx->push((VMObject*)cls);
		clsmthd->execute(ctx);
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

VMObjectArray* JVM::createObjectArray(unsigned size){
	VMObjectArray* arr = new VMObjectArray(size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMByteArray* JVM::createByteArray(unsigned size){
	VMByteArray* arr = new VMByteArray(size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMCharArray* JVM::createCharArray(unsigned size){
	VMCharArray* arr = new VMCharArray(size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMIntArray* JVM::createIntArray(unsigned size){
	VMIntArray* arr = new VMIntArray(size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMFloatArray* JVM::createFloatArray(unsigned size){
  VMFloatArray* arr = new VMFloatArray(size);
  mCreatedObjects.push_back(arr);
  return arr;
}

VMDoubleArray* JVM::createDoubleArray(unsigned size){
  VMDoubleArray* arr = new VMDoubleArray(size);
  mCreatedObjects.push_back(arr);
  return arr;
}

VMLongArray* JVM::createLongArray(unsigned size){
  VMLongArray* arr = new VMLongArray(size);
  mCreatedObjects.push_back(arr);
  return arr;
}

VMObject* JVM::createObject(VMContext* ctx, VMClass* cls){
	VMObject* obj = new VMObject(ctx, cls);
	mCreatedObjects.push_back(obj);
	return obj;
}

void JVM::initBasicClasses(VMContext* ctx){
	VMClass* sys = findClass(ctx, "java/lang/System");
	VMMethod* sysinit = sys->getMethod(sys->findMethodIndex("initializeSystemClass", "()V"));
	sysinit->execute(ctx);
	//createString(ctx, "teststring");
	VMClass* ldrcls = findClass(ctx, "java/lang/ClassLoader");
	VMMethod* mthd = ldrcls->getMethod(ldrcls->findMethodIndex("<init>", "()V"));
	VMObject* ldr = createObject(ctx, ldrcls);
	ctx->push(ldr);
	mthd->execute(ctx);
	return;
}

VMObject* JVM::createString(VMContext* ctx, const char* str){
	VMClass* cls = getVM()->findClass(ctx, "java/lang/String");
	VMObject* obj = getVM()->createObject(ctx, cls);
	ctx->push(obj);
	unsigned idx = cls->findMethodIndex("<init>", "([B)V");
	VMMethod* mthd = cls->getMethod(idx);
	VMByteArray* arr = getVM()->createByteArray(strlen(str));
	ctx->push(arr);
	arr->setData((const jbyte*)str);
	mthd->execute(ctx);
	return obj;
}

VMObject* JVM::internalizeString(const std::string& str, VMObject* strobj){
	std::map<std::string, VMObject*>::iterator iter = mInternalizedStrings.find(str);
	if (iter != mInternalizedStrings.end()){
		return iter->second;
	}
	mInternalizedStrings[str] = strobj;
	return strobj;
}
