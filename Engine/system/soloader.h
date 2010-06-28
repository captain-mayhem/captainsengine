#ifndef CGE_SOLOADER_H
#define CGE_SOLOADER_H

#include <string>

#ifdef WIN32
#include <windows.h>
#endif

namespace CGE{

typedef void* (*loadedFunc)();

class SOLoader{
public:
	SOLoader();
	SOLoader(const std::string& path, const std::string& library);
	~SOLoader();
	bool open(std::string path, std::string library);
	void addSearchPath(const std::string& path);
	loadedFunc getFunction(const char* name);
protected:
#ifdef WIN32
	HMODULE mLibrary;
#else
	void* mLibrary;
#endif
};

}

#endif
