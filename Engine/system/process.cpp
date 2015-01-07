#include "process.h"

using namespace CGE;

Process::Process(){
#ifdef WIN32
  mProcess = INVALID_HANDLE_VALUE;
#endif
}

Process::Process(std::string const& name) : mName(name){
#ifdef WIN32
  mProcess = INVALID_HANDLE_VALUE;
#endif
}

Process::~Process(){
#ifdef WIN32
  if (mProcess != INVALID_HANDLE_VALUE)
    CloseHandle(mProcess);
#endif
}

bool Process::start(){
#ifdef WIN32
  STARTUPINFO startup;
  memset(&startup, 0, sizeof(startup));
  startup.cb = sizeof(startup);

  PROCESS_INFORMATION info;
  BOOL ret = CreateProcess(mName.c_str(), (LPSTR)mArguments.c_str(), NULL, NULL, FALSE, 0, NULL, mWD.empty() ? NULL : mWD.c_str(), &startup, &info);
  mProcess = info.hProcess;
  CloseHandle(info.hThread);
  return ret != 0;
#else
  return false;
#endif
}
