#ifndef VM_LOADER_H
#define VM_LOADER_H

#include <map>

class VMClass;
class VMContext;
class VMObject;
namespace CGE{
  class Reader;
}

class VMLoader{
public:
  VMLoader(VMObject* loader);
  ~VMLoader();
  VMClass* load(VMContext* ctx, const std::string& name, CGE::Reader& rdr);
  VMClass* find(const std::string& name);
protected:
  VMObject* mLoader;
  std::map<std::string, VMClass*> mClasses;
};

#endif
