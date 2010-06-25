
#include "JVM.h"

#include "VMContext.h"
#include "ClassLoader.h"

JVM::JVM(){
  JNIInvokeInterface_::reserved0 = NULL;
  JNIInvokeInterface_::DestroyJavaVM = DestroyJavaVM;
}

JVM::~JVM(){
  for (std::list<VMContext*>::iterator iter = mThreads.begin(); iter != mThreads.end(); ++iter){
    delete *iter;
  }
  mThreads.clear();
}

VMClass* JVM::findClass(std::string name){
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
    ClassLoader* loader = new ClassLoader(name);
    loader->print();
    delete loader;
    //entry = loader->load(*this);
  }
  return entry;
  return NULL;
}
