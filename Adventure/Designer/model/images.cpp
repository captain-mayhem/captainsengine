#include "images.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include <image/loader.h>
#include <io/ZipReader.h>

namespace fs = std::filesystem;

namespace sd{

namespace{

fs::path toPath(const std::string& cp1252){
  std::string utf8 = toUtf8(cp1252);
  return fs::path(std::u8string(utf8.begin(), utf8.end()));
}

bool readFile(const fs::path& path, std::vector<char>& data){
  std::error_code ec;
  if (!fs::is_regular_file(path, ec))
    return false;
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;
  data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  return !data.empty();
}

std::string lowerExtension(const std::string& name){
  size_t dot = name.find_last_of('.');
  std::string ext = dot == std::string::npos ? "" : name.substr(dot+1);
  for (char& c : ext)
    c = (char)tolower((unsigned char)c);
  return ext;
}

CGE::Image* decode(std::vector<char>& data, const std::string& name){
  //a .pnj is a JPEG under another name
  CGE::ImageLoader::Type type = lowerExtension(name) == "pnj" ? CGE::ImageLoader::JPG : CGE::ImageLoader::determineType(name);
  if (type == CGE::ImageLoader::UNKNOWN)
    return nullptr;
  return CGE::ImageLoader::load(data.data(), (unsigned)data.size(), type);
}

//! the media pool folder that holds an image, empty if none
std::string poolFolderOf(const std::vector<PoolNode>& nodes, const std::string& name, const std::string& parent){
  for (const PoolNode& node : nodes){
    if (node.id == (int)ResourceID::Image && toLower(node.label) == toLower(name))
      return parent;
    std::string found = poolFolderOf(node.children, name, node.label);
    if (!found.empty())
      return found;
  }
  return std::string();
}

//! the bytes of a file of the project, by the ways loadProjectImage tries
bool readImageFile(const std::string& file, const std::string& folder, const std::string& poolFolder, const std::string& password, std::vector<char>& data){
  if (readFile(toPath(file), data))
    return true;
  std::string name = fileNameOf(file);
  if (folder.empty() || name.empty())
    return false;
  if (readFile(toPath(folder) / toPath(name), data))
    return true;
  //an unpacked .adz has a folder for each of the media pool's folders
  if (!poolFolder.empty() && readFile(toPath(folder) / toPath(poolFolder) / toPath(name), data))
    return true;
  fs::path gfx = toPath(folder) / "gfx.dat";
  std::error_code ec;
  if (!fs::is_regular_file(gfx, ec))
    return false;
  std::u8string gfxName = gfx.u8string();
  CGE::ZipReader zip(std::string(gfxName.begin(), gfxName.end()));
  CGE::MemReader rdr = zip.openEntry(name, password);
  if (!rdr.isWorking() || rdr.getSize() == 0)
    return false;
  data.assign(rdr.getData(), rdr.getData()+rdr.getSize());
  return true;
}

}

std::string fileNameOf(const std::string& path){
  size_t pos = path.find_last_of("/\\");
  return pos == std::string::npos ? path : path.substr(pos+1);
}

std::unique_ptr<CGE::Image> loadProjectImage(const Project& project, const std::string& folder, const std::string& name){
  const std::string* file = project.images.find(name);
  if (file == nullptr || file->empty())
    return nullptr;
  std::string poolFolder = poolFolderOf(project.mediaPool, name, std::string());
  std::vector<char> data;
  if (!readImageFile(*file, folder, poolFolder, project.zipPassword, data))
    return nullptr;
  std::unique_ptr<CGE::Image> img(decode(data, *file));
  if (!img || img->getWidth() == 0 || img->getHeight() == 0)
    return nullptr;
  if (lowerExtension(*file) == "pnj"){
    std::string alphaFile = file->substr(0, file->size()-1) + "a";
    std::vector<char> alphaData;
    std::unique_ptr<CGE::Image> alpha;
    if (readImageFile(alphaFile, folder, poolFolder, project.zipPassword, alphaData))
      alpha.reset(CGE::ImageLoader::load(alphaData.data(), (unsigned)alphaData.size(), CGE::ImageLoader::JPG));
    //the alpha is the .pna's first channel, as in the game
    if (alpha && alpha->getWidth() == img->getWidth() && alpha->getHeight() == img->getHeight() && img->getNumChannels() < 4)
      img.reset(new CGE::Image(img->getNumChannels(), img->getWidth(), img->getHeight(), img->getData(), alpha->getNumChannels(), alpha->getData()));
  }
  if (img->getNumChannels() != 4)
    img.reset(img->convertImage(4));
  return img;
}

}
