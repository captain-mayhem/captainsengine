#ifndef METHODAREA_H
#define METHODAREA_H

#include <vector>
#include <map>
#include <list>

#include <system/types.h>

class ClassLoader;
struct ClassData;

union Types{
  ClassData* mClass;
  int32 mInt;
  uint32 mUint;
  int64 mLong;
  float mFloat;
  double mDouble;
};

struct HeapClassData{
  unsigned ClassDataIndex;
};

struct ClassData{
  ClassLoader* mLoader;
  std::vector<Types> mFields;
  std::vector<uint32> mVtable;
};

struct Operation{
  char opcode;
  Types data;
};

struct MethodEntry{
  MethodEntry(std::string name, std::string signature, unsigned classindex) : mName(name), mSignature(signature), mClassindex(classindex) {}
  bool operator<(const MethodEntry& m) const{
    if (mClassindex == m.mClassindex){
      int res = strcmp(mName.c_str(), m.mName.c_str());
      if (res == 0){
        return strcmp(mSignature.c_str(), m.mSignature.c_str()) < 0;
      }
      return res < 0;
    }
    return mClassindex < m.mClassindex;
  }
  std::string mName;
  std::string mSignature;
  unsigned mClassindex;
};

typedef std::list<Operation> Codes;
typedef std::vector<Codes> Methods;

class MethodArea{
  friend class ClassLoader;
public:
  MethodArea();
  ~MethodArea();
  unsigned getClassIndex(std::string name);
  unsigned prepareMethod(std::string name, std::string signature, unsigned classindex);
  unsigned getMethodIndex(const MethodEntry& method);
  ClassData* getClassData(unsigned index);
protected:
  std::map <std::string,unsigned> mClassResolver;
  std::map <MethodEntry,unsigned> mMethodResolver;
  std::map <MethodEntry,unsigned> mFieldResolver;
  std::map <MethodEntry,unsigned> mVtableResolver;
  std::vector<ClassData*> mClassData;
  std::vector<Codes> mMethods;
  std::vector<std::string> mStrings;

  std::vector<MethodEntry> mToLoad;
};

#endif
