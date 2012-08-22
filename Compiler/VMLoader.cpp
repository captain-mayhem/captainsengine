#include "VMLoader.h"
#include "VMclass.h"

VMLoader::VMLoader(VMObject* loader) : mLoader(loader) {

}

VMLoader::~VMLoader(){
  for (std::map<std::string, VMClass*>::iterator iter = mClasses.begin(); iter != mClasses.end(); ++iter){
    delete iter->second;
  }
  mClasses.clear();
}

VMClass* VMLoader::load(VMContext* ctx, const std::string& name, CGE::Reader& rdr){
  VMClass* cls = new VMClass(ctx, rdr);
  cls->initClass(ctx, true);
  mClasses[name] = cls;
  return cls;
}

VMClass* VMLoader::find(const std::string& name){
  std::map<std::string,VMClass*>::iterator iter = mClasses.find(name);
  if (iter == mClasses.end())
    return NULL;
  return iter->second;
}
