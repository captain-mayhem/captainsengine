#include "soloader.h"

#ifdef UNIX
#include <dlfcn.h>
#endif

#include "utilities.h"


using namespace CGE;

SOLoader::SOLoader() : mLibrary(NULL){
}

SOLoader::SOLoader(const std::string& path, const std::string& library){
	open(path, library);
}

SOLoader::~SOLoader(){
#ifdef WIN32
	FreeLibrary(mLibrary);
#endif
#ifdef UNIX
	dlclose(mLibrary);
#endif
}

bool SOLoader::open(std::string path, std::string library){
#ifdef WIN32
	library += ".dll";
	CGE::Utilities::replaceWith(path, '/', '\\');
	library = path+"\\"+library;
	mLibrary = LoadLibraryEx(library.c_str(), NULL, 0);
#endif
#ifdef UNIX
	library = path+"/lib"+library+".so";
	mLibrary = dlopen(library.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
	return mLibrary != NULL;
}

loadedFunc SOLoader::getFunction(const char* name){
#ifdef WIN32
	return (loadedFunc)GetProcAddress(mLibrary, name);
#endif
#ifdef UNIX
	return (loadedFunc)dlsym(mLibrary, name);
#endif
}

void SOLoader::addSearchPath(const std::string& path){
#ifdef WIN32
	SetDllDirectory(path.c_str());
#endif
}
