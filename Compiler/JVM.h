#ifndef JVM_H
#define JVM_H

#include "jni.h"

#include <list>
#include <map>

#include <io/ZipReader.h>
#include <system/soloader.h>

#include "JavaDefs.h"

class VMClass;
class VMContext;
class VMMethod;
class VMArray;
class VMObject;

class JVM : public JNIInvokeInterface_{
public:
  JVM();
  ~JVM();
  void addThread(VMContext* thrd) {mThreads.push_back(thrd);}
	CGE::MemReader getClassFile(const std::string& filename);
  VMClass* findClass(VMContext* ctx, std::string name);
	nativeMethod findNativeMethod(const std::string& name);
	VMArray* createArray(unsigned size);
	VMObject* createObject(VMClass* cls);
protected:
	void init();

  static jint DestroyJavaVM(JavaVM* vm);

	CGE::ZipReader mRuntimeClasses;
	CGE::SOLoader mRuntime;
  std::list<VMContext*> mThreads;
  std::map <std::string,VMClass*> mClassResolver;
	std::list<VMObject*> mCreatedObjects;
};

JVM* getVM();

#endif