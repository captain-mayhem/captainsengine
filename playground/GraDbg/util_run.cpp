#include "util_run.h"

#include <string>

#ifdef WIN32
#define _WIN32_WINNT 0x0502
#include <windows.h>



void* loadLibrary(const std::string& name){
  char sysdir[256];
  GetWindowsDirectory(sysdir, 256);
  std::string dir = std::string(sysdir)+std::string("\\system32");
  //SetDllDirectory("");
  SetDllDirectory(dir.c_str());
  HMODULE module = LoadLibraryEx((dir+"\\"+name).c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
  DWORD err = GetLastError();
  //FARPROC q = GetProcAddress(module, "glVertex2f");
  //FARPROC p = GetProcAddress(module, "wglCreateContext");
  return module;
}

DLLPROC getSymbol(void* lib, const char* name){
  return (DLLPROC)GetProcAddress((HMODULE)lib, name);
}

#else
#include <dlfcn.h>

void* loadLibrary(const std::string& name){
	void* module = dlopen(("/armle/usr/lib/"+name).c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_WORLD | RTLD_GROUP);
	return module;
}

DLLPROC getSymbol(void* lib, const char* name){
	return (DLLPROC)dlsym(lib, name);
}
#endif