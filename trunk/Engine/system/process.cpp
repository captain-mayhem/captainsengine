#include "process.h"

#include <cstdlib>
#ifdef UNIX
#include <unistd.h>
#endif

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
#elif defined(UNIX)
  pid_t pid = fork();
  if (pid == 0){
    //child
    if (!mWD.empty())
      chdir(mWD.c_str());
    char* args[3];
    args[0] = (char*)mName.c_str();
    args[1] = (char*)mArguments.c_str();
    args[2] = NULL;
    execvp(mName.c_str(), args);
    _exit(-1); //execvp should not return if successful
  }
  else if (pid < 0){
    return false;
  }
  else{
    mProcess = pid;
    return true;
  }
#else
  return false;
#endif
}
