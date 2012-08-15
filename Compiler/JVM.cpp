
#include "JVM.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <system/engine.h>

#include "VMContext.h"
#include "VMclass.h"
#include "Trace.h"
#include "VMMethod.h"
#include "VMArray.h"

#define PROC_MAP_MODE
#include "Preproc.h"
#include "JavaOpcodes.h"
#undef PROC_MAP_MODE

TR_CHANNEL(Java_JVM);

static JVM* globalVM = NULL;

JVM* getVM(){
	return globalVM;
}

JVM::JVM(VMArgs* args) : mArgs(args){
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  CGE::Engine::instance()->init();
	globalVM = this;
  JNIInvokeInterface_::reserved0 = NULL;
  JNIInvokeInterface_::DestroyJavaVM = DestroyJavaVM;
	init();
}

JVM::~JVM(){
  for (std::list<VMContext*>::iterator iter = mThreads.begin(); iter != mThreads.end(); ++iter){
    delete *iter;
  }
  mThreads.clear();
	for (std::list<VMObject*>::iterator iter = mCreatedObjects.begin(); iter != mCreatedObjects.end(); ++iter){
		delete *iter;
	}
	mCreatedObjects.clear();
	globalVM = NULL;
	for (std::map<std::string,VMClass*>::iterator iter = mClassResolver.begin(); iter != mClassResolver.end(); ++iter){
		delete iter->second;
	}
	mClassResolver.clear();
  CGE::Engine::instance()->shutdown();
}

void JVM::init(){
  TR_USE(Java_JVM);
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
    //prefix[0] = 'E';
    if (!mRuntimeClasses.openFile(prefix+"/lib/rt.jar")){
      TR_BREAK("Java runtime classes not found");
    }
  }
	if (!mRuntime.open("javaruntime")){
		TR_BREAK("Java runtime not found");
	}
  Opcode::init();

  //TODO parse class path correctly
  mFilePaths.push_back(mArgs->classpath);
}

VMClass* JVM::findClass(VMContext* ctx, std::string name){
  TR_USE(Java_JVM);
	std::map<std::string,VMClass*>::iterator iter = mUninitializedClasses.find(name);
	if (iter != mUninitializedClasses.end()){
		VMClass* cls = iter->second;
		mUninitializedClasses.erase(iter);
		mClassResolver[name] = cls;
		//delayed class init
		unsigned idx = cls->findMethodIndex("<clinit>", "()V");
		VMMethod* mthd = cls->getMethod(idx);
		if (mthd){
			TR_INFO("Delayed execution of class init method");
			mthd->execute(ctx, -1);
		}
		return cls;
	}
  VMClass* entry = mClassResolver[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      entry = new VMArrayClass(name);
			mClassResolver[name] = entry;
      return entry;
    }
    else if (name.size() == 1){
      //primitive types
      return getPrimitiveClass(ctx, name);
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
		clsmthd->execute(ctx, -1);

		unsigned idx = entry->findMethodIndex("<clinit>", "()V");
		VMMethod* mthd = entry->getMethod(idx);
		if (mthd){
			TR_INFO("Found class init method");
			mthd->execute(ctx, -1);
		}
  }
  return entry;
}

std::string JVM::findClass(VMContext* ctx, VMClass* clazz){
  for (std::map<std::string,VMClass*>::iterator iter = mClassResolver.begin();
    iter != mClassResolver.end(); ++iter){
    if (iter->second == clazz)
      return iter->first;
  }
  return "";
}

VMClass* JVM::defineClass(VMContext* ctx, const std::string& name){
	std::map<std::string,VMClass*>::iterator iter = mClassResolver.find(name);
	if (iter != mClassResolver.end())
		return iter->second;
	VMClass* entry = mUninitializedClasses[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      entry = new VMArrayClass(name);
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
		clsmthd->execute(ctx, -1);
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
		clsmthd->execute(ctx, -1);
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

VMObjectArray* JVM::createObjectArray(VMContext* ctx, VMClass* cls, unsigned size){
  std::string objecttype = findClass(ctx, cls);
  objecttype.insert(0, "[L");
  objecttype.append(1, ';');
  VMClass* arrayclass = findClass(ctx, objecttype);
	VMObjectArray* arr = new VMObjectArray(ctx, arrayclass, size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMByteArray* JVM::createByteArray(VMContext* ctx, unsigned size){
  VMClass* cls = findClass(ctx, "[B");
	VMByteArray* arr = new VMByteArray(ctx, cls, size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMCharArray* JVM::createCharArray(VMContext* ctx, unsigned size){
  VMClass* cls = findClass(ctx, "[C");
	VMCharArray* arr = new VMCharArray(ctx, cls, size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMIntArray* JVM::createIntArray(VMContext* ctx, unsigned size){
  VMClass* cls = findClass(ctx, "[I");
	VMIntArray* arr = new VMIntArray(ctx, cls, size);
	mCreatedObjects.push_back(arr);
	return arr;
}

VMFloatArray* JVM::createFloatArray(VMContext* ctx, unsigned size){
  VMClass* cls = findClass(ctx, "[F");
  VMFloatArray* arr = new VMFloatArray(ctx, cls, size);
  mCreatedObjects.push_back(arr);
  return arr;
}

VMDoubleArray* JVM::createDoubleArray(VMContext* ctx, unsigned size){
  VMClass* cls = findClass(ctx, "[D");
  VMDoubleArray* arr = new VMDoubleArray(ctx, cls, size);
  mCreatedObjects.push_back(arr);
  return arr;
}

VMLongArray* JVM::createLongArray(VMContext* ctx, unsigned size){
  VMClass* cls = findClass(ctx, "[L");
  VMLongArray* arr = new VMLongArray(ctx, cls, size);
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
	sysinit->execute(ctx, -2);
	//createString(ctx, "teststring");
	/*VMClass* ldrcls = findClass(ctx, "java/lang/ClassLoader");
	VMMethod* mthd = ldrcls->getMethod(ldrcls->findMethodIndex("<init>", "()V"));
	VMObject* ldr = createObject(ctx, ldrcls);
	ctx->push(ldr);
	mthd->execute(ctx);*/
	return;
}

#ifdef WIN32
#include <windows.h>
#endif

/*
unction utf16to8(str) {
    var out, i, len, c;

    out = "";
    len = str.length;
    for(i = 0; i < len; i++) {
	c = str.charCodeAt(i);
	if ((c >= 0x0001) && (c <= 0x007F)) {
	    out += str.charAt(i);
	} else if (c > 0x07FF) {
	    out += String.fromCharCode(0xE0 | ((c >> 12) & 0x0F));
	    out += String.fromCharCode(0x80 | ((c >>  6) & 0x3F));
	    out += String.fromCharCode(0x80 | ((c >>  0) & 0x3F));
	} else {
	    out += String.fromCharCode(0xC0 | ((c >>  6) & 0x1F));
	    out += String.fromCharCode(0x80 | ((c >>  0) & 0x3F));
	}
    }
    return out;
}
*/

static int utf8to16(const char* in, unsigned short* out, unsigned outsize){
  unsigned len = (unsigned)strlen(in);
  unsigned i = 0;
  int outcount = 0;
  while (i < len){
    unsigned c = in[i++];
    switch (c >> 4){
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        if (out)
          out[outcount] = c;
        outcount++;
        break;
      case 12: case 13:{
        unsigned char2 = in[i++];
        if (out){
          out[outcount] = ((c & 0x1f) << 6) | (char2 & 0x3f);
        }
        outcount++;
        break;}
      case 14:{
        unsigned char2 = in[i++];
        unsigned char3 = in[i++];
        if (out){
          out[outcount] = ((c & 0x0f) << 12) | ((char2 & 0x3f) << 6) | (char3 & 0x3f);
        }
        outcount++;
        break;}
    }
  }
  if (out)
    out[outcount] = 0;
  return outcount+1;
}

VMObject* JVM::createString(VMContext* ctx, const char* str){
	//convert to utf16
  //int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
  int size = utf8to16(str, NULL, 0);
  unsigned short* utf16 = new unsigned short[size];
  //MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)utf16, size);
  utf8to16(str, utf16, size);
  VMClass* cls = getVM()->findClass(ctx, "java/lang/String");
	VMObject* obj = getVM()->createObject(ctx, cls);
	ctx->push(obj);
	unsigned idx = cls->findMethodIndex("<init>", "([C)V");
	VMMethod* mthd = cls->getMethod(idx);
  VMCharArray* arr = getVM()->createCharArray(ctx, size-1);
	ctx->push(arr);
	arr->setData(utf16);
	mthd->execute(ctx, -1);
  delete [] utf16;
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

void JVM::registerObject(VMObject* obj){
  mCreatedObjects.push_back(obj);
}

VMContext* JVM::createContext(VMObject* thrd){
  VMContext* ctx = new VMContext(NULL, this, thrd);
  JNIEnv_* env = new JNIEnv_(ctx);
  ctx->setJNIEnv(env);
  return ctx;
}

void JVM::destroyContext(VMContext* ctx){
  mThreads.remove(ctx);
  delete ctx;
}

std::string JVM::stringToString(VMObject* str){
  VMObject* obj = str;
  VMCharArray* ca = (VMCharArray*)obj->getObjField(1)->obj;
  std::string data;
  for (unsigned i = 0; i < ca->getLength(); ++i){
    data.append(1, (char)ca->get(i));
  }
  return data;
}
