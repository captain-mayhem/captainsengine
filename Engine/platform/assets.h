#ifndef CGE_ASSETS_H
#define CGE_ASSETS_H

#include <cge_export.h>

#include <system/types.h>

#include <memory>
#include <string>
#include <vector>

namespace CGE{

class MemReader;

//! Read-only access to the application's assets, the same on every platform
/*! A relative path names an asset: on desktop a file below the asset folder
    (assetDir in engine.ini, by default the working directory), on Android
    an asset in the APK (AAssetManager), and on the web a file in the virtual
    file system that --preload-file fills. Absolute paths (for example below
    UserDirs::getDataDir()) are always read as plain files.
    Paths use '/' as separator. */
class CGE_API Assets{
public:
  //! sets the folder relative asset paths are resolved against (desktop and web)
  static void setRoot(const std::string& dir);
  //! the folder relative asset paths are resolved against; empty means the working directory
  static const std::string& getRoot();
  //! the path of an asset as a plain file (desktop and web); empty on Android, where assets are in the APK
  static std::string getFilePath(const std::string& path);
  //! does the asset exist
  static bool exists(const std::string& path);
  //! reads a whole asset into data; false if it doesn't exist or can't be read
  static bool load(const std::string& path, std::vector<uint8>& data);
  //! reads a whole asset into a string
  static bool loadText(const std::string& path, std::string& text);
  //! reads a whole asset and returns a reader over it, or null
  static std::unique_ptr<MemReader> open(const std::string& path);
  //! the files (not the folders) in an asset folder
  static std::vector<std::string> listFiles(const std::string& dir);
  //! is the path absolute (read as a plain file on every platform)
  static bool isAbsolute(const std::string& path);
};

}

#endif
