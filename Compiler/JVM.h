#ifndef JVM_H
#define JVM_H

#include "jni.h"

#include <list>
#include <map>

#include <io/ZipReader.h>
#include <system/soloader.h>

#include "JavaDefs.h"
#include "VMArray.h"

class VMClass;
class VMContext;
class VMMethod;
class VMObject;

class JVM : public JNIInvokeInterface_{
public:
  JVM();
  ~JVM();
	void initBasicClasses(VMContext* ctx);
  void addThread(VMContext* thrd) {mThreads.push_back(thrd);}
	CGE::MemReader getClassFile(const std::string& filename);
  VMClass* findClass(VMContext* ctx, std::string name);
	VMClass* defineClass(VMContext* ctx, const std::string& name);
	VMClass* getPrimitiveClass(VMContext* ctx, std::string name);
	nativeMethod findNativeMethod(const std::string& name);
	VMObjectArray* createObjectArray(VMContext* ctx, VMClass* cls, unsigned size);
	VMByteArray* createByteArray(VMContext* ctx, unsigned size);
	VMCharArray* createCharArray(VMContext* ctx, unsigned size);
	VMIntArray* createIntArray(VMContext* ctx, unsigned size);
  VMFloatArray* createFloatArray(VMContext* ctx, unsigned size);
  VMDoubleArray* createDoubleArray(VMContext* ctx, unsigned size);
  VMLongArray* createLongArray(VMContext* ctx, unsigned size);
	VMObject* createObject(VMContext* ctx, VMClass* cls);
	VMObject* createString(VMContext* ctx, const char* str);
	VMObject* internalizeString(const std::string& str, VMObject* strobj);
protected:
	void init();

  static jint DestroyJavaVM(JavaVM* vm);

	CGE::ZipReader mRuntimeClasses;
	CGE::SOLoader mRuntime;
  std::list<VMContext*> mThreads;
  std::map <std::string,VMClass*> mClassResolver;
	std::map <std::string,VMClass*> mUninitializedClasses;
	std::list<VMObject*> mCreatedObjects;
	std::map<std::string, VMObject*> mInternalizedStrings;
};

JVM* getVM();

#endif