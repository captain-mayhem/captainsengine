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
  virtual ~VMLoader();
  VMClass* load(VMContext* ctx, const std::string& name, CGE::Reader& rdr);
  virtual VMClass* find(VMContext* ctx, const std::string& name);
  VMClass* get(const std::string& name);
  VMObject* getLoaderObject() {return mLoader;}
protected:
  VMObject* mLoader;
  std::map<std::string, VMClass*> mClasses;
};

#include <vector>

#include <io/ZipReader.h>
#include <system/soloader.h>

#include "JavaDefs.h"

struct VMArgs;

class BootstrapLoader : public VMLoader{
public:
  BootstrapLoader(VMArgs* args);
  virtual VMClass* find(VMContext* ctx, const std::string& name){
    return find(ctx, name, true);
  }
  VMClass* loadWithoutInit(VMContext* ctx, const std::string& name){
    return find(ctx, name, false);
  }
  nativeMethod findNativeMethod(const std::string& name);
  VMClass* getPrimitiveClass(VMContext* ctx, std::string name);
private:
  VMClass* find(VMContext* ctx, const std::string& name, bool initClass);
  CGE::Reader* filenameToReader(const std::string& filename);
  CGE::MemReader getClassFile(const std::string& filename);

  std::map<std::string, VMClass*> mUninitializedClasses;

  CGE::ZipReader mRuntimeClasses;
  std::vector<std::string> mFilePaths;
  CGE::SOLoader mRuntime;
};

#endif
