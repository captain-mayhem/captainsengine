#include "assets.h"

#include <cstdio>

#include "../io/MemReader.h"
#include "../io/Tracing.h"
#include "../system/file.h"
#include "platform_internal.h"

#ifdef ANDROID
#include <android/asset_manager.h>
#endif

using namespace CGE;

TR_CHANNEL(CGE_Assets);

namespace{

std::string assetRoot;
#ifdef ANDROID
AAssetManager* assetManager = nullptr;
#endif

//! a MemReader that owns the data it reads
class AssetReader : public MemReader{
public:
  explicit AssetReader(std::vector<uint8>&& data) : mData(std::move(data)){
    mStart = (char*)mData.data();
    mEnd = mStart + mData.size();
    mCurrent = mStart;
  }
private:
  std::vector<uint8> mData;
};

bool loadPlainFile(const std::string& filename, std::vector<uint8>& data){
  FILE* f = fopen(filename.c_str(), "rb");
  if (!f)
    return false;
  bool ok = fseek(f, 0, SEEK_END) == 0;
  long size = ok ? ftell(f) : -1;
  ok = size >= 0 && fseek(f, 0, SEEK_SET) == 0;
  if (ok){
    data.resize((size_t)size);
    ok = size == 0 || fread(data.data(), 1, (size_t)size, f) == (size_t)size;
  }
  fclose(f);
  if (!ok)
    data.clear();
  return ok;
}

#ifdef ANDROID
//! AAssetManager wants "dir/file", without "./" and with '/' only
std::string apkPath(const std::string& path){
  std::string ret = path;
  for (char& ch : ret){
    if (ch == '\\')
      ch = '/';
  }
  while (ret.compare(0, 2, "./") == 0)
    ret.erase(0, 2);
  while (!ret.empty() && ret.back() == '/')
    ret.pop_back();
  return ret;
}
#endif

}

void Assets::setRoot(const std::string& dir){
  assetRoot = dir;
  while (assetRoot.size() > 1 && (assetRoot.back() == '/' || assetRoot.back() == '\\'))
    assetRoot.pop_back();
}

const std::string& Assets::getRoot(){
  return assetRoot;
}

bool Assets::isAbsolute(const std::string& path){
  if (path.empty())
    return false;
  if (path[0] == '/' || path[0] == '\\')
    return true;
  //drive letter
  return path.size() > 1 && path[1] == ':';
}

std::string Assets::getFilePath(const std::string& path){
  if (isAbsolute(path))
    return path;
#ifdef ANDROID
  return "";
#else
  if (assetRoot.empty())
    return path;
  return assetRoot + "/" + path;
#endif
}

bool Assets::exists(const std::string& path){
#ifdef ANDROID
  if (!isAbsolute(path)){
    if (!assetManager)
      return false;
    AAsset* asset = AAssetManager_open(assetManager, apkPath(path).c_str(), AASSET_MODE_STREAMING);
    if (!asset)
      return false;
    AAsset_close(asset);
    return true;
  }
#endif
  return Filesystem::exists(getFilePath(path));
}

bool Assets::load(const std::string& path, std::vector<uint8>& data){
  TR_USE(CGE_Assets);
  data.clear();
#ifdef ANDROID
  if (!isAbsolute(path)){
    AAsset* asset = assetManager ? AAssetManager_open(assetManager, apkPath(path).c_str(), AASSET_MODE_BUFFER) : nullptr;
    if (!asset){
      TR_DEBUG("asset %s not found", path.c_str());
      return false;
    }
    const void* buffer = AAsset_getBuffer(asset);
    off_t size = AAsset_getLength(asset);
    if (buffer)
      data.assign((const uint8*)buffer, (const uint8*)buffer + size);
    AAsset_close(asset);
    return buffer != nullptr;
  }
#endif
  if (!loadPlainFile(getFilePath(path), data)){
    TR_DEBUG("asset %s not found", path.c_str());
    return false;
  }
  return true;
}

bool Assets::loadText(const std::string& path, std::string& text){
  std::vector<uint8> data;
  if (!load(path, data)){
    text.clear();
    return false;
  }
  text.assign(data.begin(), data.end());
  return true;
}

std::unique_ptr<MemReader> Assets::open(const std::string& path){
  std::vector<uint8> data;
  if (!load(path, data))
    return nullptr;
  return std::make_unique<AssetReader>(std::move(data));
}

std::vector<std::string> Assets::listFiles(const std::string& dir){
#ifdef ANDROID
  if (!isAbsolute(dir)){
    std::vector<std::string> result;
    AAssetDir* adir = assetManager ? AAssetManager_openDir(assetManager, apkPath(dir).c_str()) : nullptr;
    if (!adir)
      return result;
    while (const char* name = AAssetDir_getNextFileName(adir))
      result.push_back(name);
    AAssetDir_close(adir);
    return result;
  }
#endif
  std::string path = getFilePath(dir);
  return Filesystem::getFiles(path.empty() ? "." : path);
}

#ifdef ANDROID
namespace CGE{
namespace PlatformInternal{
void setAssetManager(AAssetManager* mgr){
  assetManager = mgr;
}
}
}
#endif
