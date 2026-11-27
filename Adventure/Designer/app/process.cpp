#include "process.h"

#include <filesystem>

#if defined(WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace sd{

namespace{

fs::path toPath(const std::string& utf8){
  return fs::path(std::u8string(utf8.begin(), utf8.end()));
}

#if defined(WIN32)
//! a command line argument quoted as CommandLineToArgvW reads it
std::wstring quote(const std::wstring& arg){
  if (!arg.empty() && arg.find_first_of(L" \t\"") == std::wstring::npos)
    return arg;
  std::wstring out = L"\"";
  size_t backslashes = 0;
  for (wchar_t c : arg){
    if (c == L'\\'){
      ++backslashes;
      continue;
    }
    if (c == L'"')
      out.append(backslashes*2+1, L'\\');
    else
      out.append(backslashes, L'\\');
    backslashes = 0;
    out += c;
  }
  out.append(backslashes*2, L'\\');
  out += L'"';
  return out;
}

std::wstring commandLine(const std::string& program, const std::vector<std::string>& args){
  std::wstring cmd = quote(toPath(program).wstring());
  for (const std::string& arg : args)
    cmd += L" " + quote(toPath(arg).wstring());
  return cmd;
}

std::string lastError(){
  DWORD err = GetLastError();
  char buf[256] = "";
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, 0, buf, sizeof(buf), nullptr);
  return buf;
}
#else
std::vector<char*> argv(const std::string& program, const std::vector<std::string>& args){
  std::vector<char*> ret;
  ret.push_back(const_cast<char*>(program.c_str()));
  for (const std::string& arg : args)
    ret.push_back(const_cast<char*>(arg.c_str()));
  ret.push_back(nullptr);
  return ret;
}
#endif

}

bool startProgram(const std::string& program, const std::vector<std::string>& args, const std::string& folder, std::string* error){
#if defined(WIN32)
  std::wstring cmd = commandLine(program, args);
  std::wstring dir = toPath(folder).wstring();
  STARTUPINFOW si = {};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = {};
  if (!CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, FALSE, 0, nullptr, dir.empty() ? nullptr : dir.c_str(), &si, &pi)){
    if (error)
      *error = "Can't start " + program + ": " + lastError();
    return false;
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return true;
#else
  //a pipe tells whether exec worked: it closes on exec, or gets errno
  int fds[2];
  if (pipe(fds) != 0){
    if (error)
      *error = std::string("Can't start ") + program + ": " + strerror(errno);
    return false;
  }
  std::vector<char*> args_ = argv(program, args);
  //made before fork, where allocating isn't safe
  const char* libs = getenv("LD_LIBRARY_PATH");
  std::string libPath = folder + (libs && *libs ? std::string(":") + libs : std::string());
  pid_t pid = fork();
  if (pid == 0){
    close(fds[0]);
    //the grandchild runs the program, so that no child waits to be reaped
    if (fork() != 0)
      _exit(0);
    setsid();
    fcntl(fds[1], F_SETFD, FD_CLOEXEC);
    if (!folder.empty()){
      if (chdir(folder.c_str()) != 0){
        int err = errno;
        if (write(fds[1], &err, sizeof(err)) < 0){}
        _exit(127);
      }
      setenv("LD_LIBRARY_PATH", libPath.c_str(), 1);
    }
    execv(program.c_str(), args_.data());
    int err = errno;
    if (write(fds[1], &err, sizeof(err)) < 0){}
    _exit(127);
  }
  close(fds[1]);
  if (pid < 0){
    close(fds[0]);
    if (error)
      *error = std::string("Can't start ") + program + ": " + strerror(errno);
    return false;
  }
  waitpid(pid, nullptr, 0);
  int err = 0;
  ssize_t got = read(fds[0], &err, sizeof(err));
  close(fds[0]);
  if (got == (ssize_t)sizeof(err)){
    if (error)
      *error = std::string("Can't start ") + program + ": " + strerror(err);
    return false;
  }
  return true;
#endif
}

int runProgram(const std::string& program, const std::vector<std::string>& args, std::string& output){
  output.clear();
#if defined(WIN32)
  SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
  HANDLE readPipe = nullptr, writePipe = nullptr;
  if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
    return -1;
  SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
  std::wstring cmd = commandLine(program, args);
  STARTUPINFOW si = {};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = writePipe;
  si.hStdError = writePipe;
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  PROCESS_INFORMATION pi = {};
  BOOL started = CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
  CloseHandle(writePipe);
  if (!started){
    CloseHandle(readPipe);
    output = lastError();
    return -1;
  }
  //both outputs in one pipe, read until the program closes it
  char buf[4096];
  DWORD got = 0;
  while (ReadFile(readPipe, buf, sizeof(buf), &got, nullptr) && got > 0)
    output.append(buf, got);
  CloseHandle(readPipe);
  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code = 0;
  GetExitCodeProcess(pi.hProcess, &code);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return (int)code;
#else
  int fds[2];
  if (pipe(fds) != 0)
    return -1;
  std::vector<char*> args_ = argv(program, args);
  pid_t pid = fork();
  if (pid == 0){
    close(fds[0]);
    dup2(fds[1], 1);
    dup2(fds[1], 2);
    close(fds[1]);
    execv(program.c_str(), args_.data());
    _exit(127);
  }
  close(fds[1]);
  if (pid < 0){
    close(fds[0]);
    return -1;
  }
  char buf[4096];
  ssize_t got;
  while ((got = read(fds[0], buf, sizeof(buf))) > 0)
    output.append(buf, (size_t)got);
  close(fds[0]);
  int status = 0;
  waitpid(pid, &status, 0);
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

std::string programFolder(){
#if defined(WIN32)
  wchar_t buf[MAX_PATH*4];
  DWORD len = GetModuleFileNameW(nullptr, buf, (DWORD)(sizeof(buf)/sizeof(buf[0])));
  fs::path exe(std::wstring(buf, len));
#else
  char buf[4096];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
  fs::path exe(std::string(buf, len > 0 ? (size_t)len : 0));
#endif
  std::u8string str = exe.parent_path().u8string();
  return std::string(str.begin(), str.end());
}

}
