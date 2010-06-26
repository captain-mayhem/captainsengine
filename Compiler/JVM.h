#ifndef JVM_H
#define JVM_H

#include "jni.h"

#include <list>
#include <map>

#include <io/ZipReader.h>

class VMClass;
class VMContext;

class JVM : public JNIInvokeInterface_{
public:
  JVM();
  ~JVM();
  void addThread(VMContext* thrd) {mThreads.push_back(thrd);}
	CGE::MemReader getClassFile(const std::string& filename);
  VMClass* findClass(std::string name);
protected:
	void init();

  static jint DestroyJavaVM(JavaVM* vm);

	CGE::ZipReader mRuntimeClasses;
  std::list<VMContext*> mThreads;
  std::map <std::string,VMClass*> mClassResolver;
};

JVM* getVM();

#endif