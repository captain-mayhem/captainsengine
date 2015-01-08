#ifndef CGE_PROCESS_H
#define CGE_PROCESS_H

#include <string>

#ifdef WIN32
#include <Windows.h>
#endif

namespace CGE{

class Process{
public:
  Process();
  Process(std::string const& name);
  ~Process();
  bool start();
  void setExecutable(std::string const& exe) { mName = exe; }
  void setArguments(std::string const& name) { mArguments = name; }
  void setWorkingDirectory(std::string const& dir) { mWD = dir; }
private:
  std::string mName;
  std::string mArguments;
  std::string mWD;
#ifdef WIN32
  HANDLE mProcess;
#endif
#ifdef UNIX
  pid_t mProcess;
#endif
};

}

#endif
