//  ==================================================================
// |                         Hero Engine                              |
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
#endif
#ifdef UNIX
#include <pthread.h>
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
  return result;
#endif
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
  return result;
#endif
}