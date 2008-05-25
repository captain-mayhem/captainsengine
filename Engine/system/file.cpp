//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: fiel.cpp                                                   |
//  ==================================================================

#include "file.h"

#include "engine.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#endif
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

using namespace System;
using std::string;
using std::vector;

//get all directories in a directory
std::vector<std::string> Filesystem::getDirectories(const std::string& path){
  vector<string> result;
#ifdef WIN32
  string tmp = path;
  tmp.append("/*");
  WIN32_FIND_DATA fileData;
  HANDLE hFind;
  hFind = FindFirstFile(tmp.c_str(), &fileData);
  if (hFind == INVALID_HANDLE_VALUE){
    return result;
  }
  if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
    result.push_back(fileData.cFileName);
  }
  while(FindNextFile(hFind, &fileData)){
    if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
      result.push_back(fileData.cFileName);
    }
  }
  FindClose(hFind);
#endif
#ifdef UNIX
  DIR* dir = opendir(path.c_str());
  if (!dir)
    return result;
  struct dirent* entry;
  while((entry = readdir(dir))){
    struct stat st;
    string tmp = path+"/"+entry->d_name;
    stat(tmp.c_str(), &st);
    if ((st.st_mode & S_IFDIR)){
      result.push_back(entry->d_name);
    }
  }
  closedir(dir);
#endif

  return result;
}

//get all directories in a directory
std::vector<std::string> Filesystem::getFiles(const std::string& path){
  vector<string> result;
#ifdef WIN32
  string tmp = path;
  tmp.append("/*");
  WIN32_FIND_DATA fileData;
  HANDLE hFind;
  hFind = FindFirstFile(tmp.c_str(), &fileData);
  if (hFind == INVALID_HANDLE_VALUE){
    return result;
  }
  if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
    result.push_back(fileData.cFileName);
  }
  while(FindNextFile(hFind, &fileData)){
    if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
      result.push_back(fileData.cFileName);
    }
  }
  FindClose(hFind);
#endif
#ifdef UNIX
  DIR* dir = opendir(path.c_str());
  if (!dir)
    return result;
  struct dirent* entry;
  while((entry = readdir(dir))){
    struct stat st;
    string tmp = path+"/"+entry->d_name;
    stat(tmp.c_str(), &st);
    if (!(st.st_mode & S_IFDIR)){
      result.push_back(entry->d_name);
    }
  }
  closedir(dir);
#endif
  return result;
}

// get the working directory
std::string Filesystem::getCwd(){
  char cwd[2048];
#ifdef WIN32
  _getcwd(cwd, 2048);
#endif
#ifdef UNIX
  getcwd(cwd, 2048);
#endif
  return string(cwd);
}

// change directory
bool Filesystem::changeDir(const std::string& path){
#ifdef WIN32
  return SetCurrentDirectory(path.c_str()) == TRUE ? true : false;
#endif
#ifdef UNIX
  return chdir(path.c_str()) == 0 ? true : false;
#endif
}

