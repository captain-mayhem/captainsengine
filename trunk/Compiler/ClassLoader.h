#ifndef CLASSLOADER_H
#define CLASSLOADER_H

#include <string>

#include "JavaDefs.h"
#include "MethodArea.h"

class ClassLoader{
public:
  ClassLoader(std::string filename);
  ~ClassLoader();
  int load(MethodArea& area);
  unsigned resolveMethod(MethodArea& area, const MethodEntry& method);
  void print();
protected:
  unsigned resolveField(MethodArea& area, const MethodEntry& field);
  unsigned resolveVtable(MethodArea& area, const MethodEntry& method);
  void resolveMethods(MethodArea& area, unsigned classidx);
  Java::ClassFile mClass;
  std::string mFilename;
  static void* mRuntime;
};

#endif
