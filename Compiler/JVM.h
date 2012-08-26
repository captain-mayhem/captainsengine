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
class VMLoader;
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
  VMContext* createContext(VMObject* thrd);
  void destroyContext(VMContext* ctx);
	VMClass* getPrimitiveClass(VMContext* ctx, std::string name);
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
  VMArgs* getArguments() {return mArgs;}
  static std::string stringToString(VMObject* str);
  static int utf8to16(const char* in, unsigned short* out, unsigned outsize);
  VMLoader* getLoader(VMObject* loader);
protected:
	void init();

  static jint DestroyJavaVM(JavaVM* vm);

  VMArgs* mArgs;

  std::list<VMContext*> mThreads;
  std::map <VMObject*,VMLoader*> mLoaders;
	std::list<VMObject*> mCreatedObjects;
	std::map<std::string, VMObject*> mInternalizedStrings;
};

JVM* getVM();

#endif
