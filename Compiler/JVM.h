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
class VMObject;
template<typename T> class VMArray;
typedef VMArray<VMObject*> VMObjectArray;
typedef VMArray<jbyte> VMByteArray;
typedef VMArray<jchar> VMCharArray;
typedef VMArray<jint> VMIntArray;
typedef VMArray<jfloat> VMFloatArray;
typedef VMArray<jdouble> VMDoubleArray;
typedef VMArray<jlong> VMLongArray;

struct VMArgs : public JDK1_1InitArgs{

};

class JVM : public JNIInvokeInterface_{
public:
  JVM(VMArgs* args);
  ~JVM();
	void initBasicClasses(VMContext* ctx);
  void addThread(VMContext* thrd) {mThreads.push_back(thrd);}
	CGE::MemReader getClassFile(const std::string& filename);
  VMClass* findClass(VMContext* ctx, std::string name);
  std::string findClass(VMContext* ctx, VMClass* clazz);
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
  void registerObject(VMObject* obj);
  std::vector<std::string>& getFilePaths() {return mFilePaths;}
protected:
	void init();

  static jint DestroyJavaVM(JavaVM* vm);

  VMArgs* mArgs;

	CGE::ZipReader mRuntimeClasses;
  std::vector<std::string> mFilePaths;
	CGE::SOLoader mRuntime;
  std::list<VMContext*> mThreads;
  std::map <std::string,VMClass*> mClassResolver;
	std::map <std::string,VMClass*> mUninitializedClasses;
	std::list<VMObject*> mCreatedObjects;
	std::map<std::string, VMObject*> mInternalizedStrings;
};

JVM* getVM();

#endif
