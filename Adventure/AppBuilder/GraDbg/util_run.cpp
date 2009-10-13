#include "util_run.h"

#define _WIN32_WINNT 0x0502
#include <windows.h>

#include <string>

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