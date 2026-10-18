#include "userdirs.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

#include "../io/Tracing.h"
#include "platform_internal.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#endif
#if defined(UNIX) && !defined(ANDROID)
#include <pwd.h>
#include <unistd.h>
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace CGE;

TR_CHANNEL(CGE_UserDirs);

namespace{

std::string appName;
std::string defaultAppName = "cge";
#ifdef ANDROID
std::string androidDataPath;
#endif

std::string withApp(const std::string& base){
  const std::string& name = UserDirs::getAppName();
  if (base.empty())
    return name;
  return base + "/" + name;
}

std::string ensureDir(const std::string& dir){
  TR_USE(CGE_UserDirs);
  std::error_code err;
  std::filesystem::create_directories(std::filesystem::path(dir), err);
  if (err)
    TR_ERROR("cannot create %s: %s", dir.c_str(), err.message().c_str());
  return dir;
}

#if defined(UNIX) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
std::string homeDir(){
  const char* home = getenv("HOME");
  if (home && home[0] != '\0')
    return home;
  struct passwd* pw = getpwuid(getuid());
  if (pw && pw->pw_dir)
    return pw->pw_dir;
  return ".";
}

//! $var if it is set to an absolute path, otherwise ~/fallback (XDG Base Directory spec)
std::string xdgDir(const char* var, const char* fallback){
  const char* dir = getenv(var);
  if (dir && dir[0] == '/')
    return dir;
  return homeDir() + "/" + fallback;
}
#endif

#ifdef WIN32
std::string roamingAppData(){
  TR_USE(CGE_UserDirs);
  PWSTR wpath = nullptr;
  std::string ret;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &wpath))){
    //the engine uses the narrow (ANSI) file functions
    int len = WideCharToMultiByte(CP_ACP, 0, wpath, -1, nullptr, 0, nullptr, nullptr);
    if (len > 1){
      ret.resize(len - 1);
      WideCharToMultiByte(CP_ACP, 0, wpath, -1, &ret[0], len, nullptr, nullptr);
    }
  }
  CoTaskMemFree(wpath);
  if (ret.empty()){
    TR_ERROR("cannot determine the AppData folder");
    ret = ".";
  }
  return ret;
}
#endif

}

void UserDirs::setAppName(const std::string& name){
  appName = name;
}

const std::string& UserDirs::getAppName(){
  return appName.empty() ? defaultAppName : appName;
}

std::string UserDirs::getDataDir(){
#if defined(WIN32)
  return ensureDir(withApp(roamingAppData()));
#elif defined(ANDROID)
  return ensureDir(androidDataPath.empty() ? std::string(".") : androidDataPath);
#elif defined(__EMSCRIPTEN__)
  return ensureDir(withApp("/IDBFS"));
#else
  return ensureDir(withApp(xdgDir("XDG_DATA_HOME", ".local/share")));
#endif
}

std::string UserDirs::getConfigDir(){
#if defined(WIN32) || defined(ANDROID) || defined(__EMSCRIPTEN__)
  return getDataDir();
#else
  return ensureDir(withApp(xdgDir("XDG_CONFIG_HOME", ".config")));
#endif
}

void UserDirs::commit(){
#ifdef __EMSCRIPTEN__
  EM_ASM(
    FS.syncfs(false, function(err){
      if (err)
        console.error("CGE: saving to IndexedDB failed", err);
    });
  );
#endif
}

namespace CGE{
namespace PlatformInternal{

void setDefaultAppName(const std::string& name){
  if (!name.empty())
    defaultAppName = name;
}

#ifdef ANDROID
void setAndroidDataPath(const char* path){
  androidDataPath = path ? path : "";
}
#endif

}
}
