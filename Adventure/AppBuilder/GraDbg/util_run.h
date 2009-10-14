#ifndef UTIL_RUN_H
#define UTIL_RUN_H

#include <string>

#ifdef WIN32
#ifdef GraDbgRuntime_EXPORTS 
#define IMPEXP __declspec(dllexport)
#else
#define IMPEXP __declspec(dllimport)
#endif
#else
#define IMPEXP
#endif

typedef int (*DLLPROC)();
void* loadLibrary(const std::string& name);
DLLPROC getSymbol(void* lib, const char* name);

#endif