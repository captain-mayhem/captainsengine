
#include "MethodArea.h"

#include "ClassLoader.h"

MethodArea::MethodArea(){
  mClassData.push_back(new ClassData());
  mMethods.push_back(Codes());
  mMethods.push_back(Codes());
}

MethodArea::~MethodArea(){
  for (unsigned i = 0; i < mClassData.size(); ++i){
    delete mClassData[i];
  }
}

unsigned MethodArea::getClassIndex(std::string name){
  unsigned entry = mClassResolver[name];
  if (entry == 0){
    //array functions
    if (name[0] == '['){
      ClassData* clsdat = new ClassData();
      clsdat->mLoader = NULL;
      mClassData.push_back(clsdat);
      entry = mClassData.size()-1;
      mClassResolver[name] = entry;
      return entry;
    }
    Java::ClassFile* clfile = new Java::ClassFile();
    ClassLoader* loader = new ClassLoader(name);
    loader->print();
    entry = loader->load(*this);
  }
  return entry;
}

unsigned MethodArea::getMethodIndex(const MethodEntry& method){
  unsigned idx = mMethodResolver[method];
  if (idx == 0){
    ClassLoader* loader = mClassData[method.mClassindex]->mLoader;
    if (loader == NULL){
      idx = 1; //array method
    }
    else{
      idx = loader->resolveMethod(*this, method);
    }
  }
  return idx;
}

ClassData* MethodArea::getClassData(unsigned index){
  return NULL;
}

unsigned MethodArea::prepareMethod(std::string name, std::string signature, unsigned classindex){
  //ClassLoader* loader = mClassData[classindex].mLoader;
  MethodEntry me(name, signature, classindex);
  return getMethodIndex(me);
}