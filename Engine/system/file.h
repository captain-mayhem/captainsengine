//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: file.h                                                     |
//  ==================================================================

#ifndef FILE_H
#define FILE_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifdef UNIX
#include <pthread.h>
#endif

#include <vector>
#include <string>

namespace System {
  
class Filesystem{
public:
  //! get all directories in a directory
  static std::vector<std::string> getDirectories(const std::string& path);
  //! get all non-directory files in a directory
  static std::vector<std::string> getFiles(const std::string& path);
  //! get the working directory
  static std::string getCwd();
  //! change directory
  static bool changeDir(const std::string& path);
#ifdef UNDER_CE
  //! working directory emulation for Windows CE
  static std::string cwd_;
#endif
};

}

#endif

