#ifndef VM_LOADER_H
#define VM_LOADER_H

#include <map>
#include <list>

#include "JavaDefs.h"

class VMClass;
class VMContext;
class VMObject;
namespace CGE{
  class Reader;
  class SOLoader;
}

class VMLoader{
public:
  VMLoader(VMObject* loader);
  virtual ~VMLoader();
  VMClass* load(VMContext* ctx, const std::string& name, CGE::Reader& rdr);
  virtual VMClass* find(VMContext* ctx, const std::string& name);
  virtual nativeMethod findNativeMethod(const std::string& name);
  VMClass* get(const std::string& name);
  VMObject* getLoaderObject() {return mLoader;}
  jlong addLibrary(const std::string& name);
protected:
  VMObject* mLoader;
  std::map<std::string, VMClass*> mClasses;
  std::list<CGE::SOLoader*> mLibs;
};

#include <vector>

#include <io/ZipReader.h>
#include <system/soloader.h>

struct VMArgs;

class BootstrapLoader : public VMLoader{
public:
  BootstrapLoader(VMArgs* args);
  virtual VMClass* find(VMContext* ctx, const std::string& name){
    return find(ctx, name, true);
  }
  virtual nativeMethod findNativeMethod(const std::string& name);
  VMClass* loadWithoutInit(VMContext* ctx, const std::string& name){
    return find(ctx, name, false);
  }
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
