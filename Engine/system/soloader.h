#ifndef CGE_SOLOADER_H
#define CGE_SOLOADER_H

#include <string>

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#include <windows.h>
#endif

namespace CGE{

typedef void* (*loadedFunc)();

class SOLoader{
public:
	SOLoader();
	SOLoader(const std::string& path, const std::string& library);
	~SOLoader();
  bool open(const std::string& library) {return open("", library);}
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
