#ifndef JVM_H
#define JVM_H

#include "jni.h"

#include <list>
#include <map>

class VMClass;
class VMContext;

class JVM : public JNIInvokeInterface_{
public:
  JVM();
  ~JVM();
  void addThread(VMContext* thrd) {mThreads.push_back(thrd);}
  VMClass* findClass(std::string name);
protected:
  static jint DestroyJavaVM(JavaVM* vm);

  std::list<VMContext*> mThreads;
  std::map <std::string,VMClass*> mClassResolver;
};

#endif