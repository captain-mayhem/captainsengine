#include "images.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

#include <png.h>

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

//! the media pool folder that holds a resource, empty if none
std::string poolFolderOf(const std::vector<PoolNode>& nodes, ResourceID kind, const std::string& name, const std::string& parent){
  for (const PoolNode& node : nodes){
    if (node.id == (int)kind && toLower(node.label) == toLower(name))
      return parent;
    std::string found = poolFolderOf(node.children, kind, name, node.label);
    if (!found.empty())
      return found;
  }
  return std::string();
}

//! the bytes of a file of the project, by the ways loadProjectImage tries
bool readImageFile(const std::string& file, const std::string& folder, const std::string& poolFolder, const std::string& password, std::vector<char>& data, const char* dat = "gfx.dat"){
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
  fs::path gfx = toPath(folder) / dat;
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

std::string findMediaFile(const Project& project, const std::string& folder, ResourceID kind, const std::string& name){
  const NamedList<std::string>* files = nullptr;
  switch (kind){
    case ResourceID::Image: files = &project.images; break;
    case ResourceID::Sound: files = &project.sounds; break;
    case ResourceID::Music: files = &project.music; break;
    case ResourceID::Video: files = &project.videos; break;
    default: return std::string();
  }
  const std::string* file = files->find(name);
  if (file == nullptr || file->empty())
    return std::string();
  std::error_code ec;
  if (fs::is_regular_file(toPath(*file), ec))
    return *file;
  std::string fname = fileNameOf(*file);
  if (folder.empty() || fname.empty())
    return std::string();
  std::vector<fs::path> tries{toPath(folder) / toPath(fname)};
  std::string poolFolder = poolFolderOf(project.mediaPool, kind, name, std::string());
  if (!poolFolder.empty())
    tries.push_back(toPath(folder) / toPath(poolFolder) / toPath(fname));
  for (const fs::path& path : tries){
    if (fs::is_regular_file(path, ec)){
      std::u8string u8 = path.u8string();
      return fromUtf8(std::string(u8.begin(), u8.end()));
    }
  }
  return std::string();
}

bool readMediaFile(const Project& project, const std::string& folder, ResourceID kind, const std::string& name, std::vector<char>& data){
  const char* dat = nullptr;
  const NamedList<std::string>* files = nullptr;
  switch (kind){
    case ResourceID::Image: files = &project.images; dat = "gfx.dat"; break;
    case ResourceID::Sound: files = &project.sounds; dat = "sfx.dat"; break;
    case ResourceID::Music: files = &project.music; dat = "music.dat"; break;
    case ResourceID::Video: files = &project.videos; dat = "movie.dat"; break;
    default: return false;
  }
  const std::string* file = files->find(name);
  if (file == nullptr || file->empty())
    return false;
  return readImageFile(*file, folder, poolFolderOf(project.mediaPool, kind, name, std::string()), project.zipPassword, data, dat);
}

std::unique_ptr<CGE::Image> loadProjectImage(const Project& project, const std::string& folder, const std::string& name){
  const std::string* file = project.images.find(name);
  if (file == nullptr || file->empty())
    return nullptr;
  std::string poolFolder = poolFolderOf(project.mediaPool, ResourceID::Image, name, std::string());
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

bool cropTransparentImage(const std::string& file, std::string* error){
  namespace fs = std::filesystem;
  fs::path path(std::u8string(file.begin(), file.end()));
  std::ifstream in(path, std::ios::binary);
  std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  std::unique_ptr<CGE::Image> image(data.empty() ? nullptr : CGE::ImageLoader::load(data.data(), (unsigned)data.size(), CGE::ImageLoader::PNG));
  if (!image){
    if (error)
      *error = "Can't read " + file;
    return false;
  }
  //only an image with alpha has a transparent border
  if (!image->hasAlpha())
    return true;
  const int width = (int)image->getWidth();
  const int height = (int)image->getHeight();
  const unsigned char* pixels = image->getData();
  int minx = width, miny = height, maxx = -1, maxy = -1;
  for (int y = 0; y < height; ++y){
    for (int x = 0; x < width; ++x){
      if (pixels[(y*width+x)*4+3] == 0)
        continue;
      minx = std::min(minx, x);
      maxx = std::max(maxx, x);
      miny = std::min(miny, y);
      maxy = std::max(maxy, y);
    }
  }
  //nothing to cut, or nothing left (the C# editor fails on that)
  if (maxx < 0 || (minx == 0 && miny == 0 && maxx == width-1 && maxy == height-1))
    return true;
  const int w = maxx-minx+1;
  const int h = maxy-miny+1;
  std::vector<unsigned char> cropped((size_t)w*h*4);
  for (int y = 0; y < h; ++y)
    std::copy_n(pixels + ((size_t)(y+miny)*width+minx)*4, (size_t)w*4, cropped.data() + (size_t)y*w*4);
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop info = png ? png_create_info_struct(png) : nullptr;
  std::vector<char> out;
  if (!info || setjmp(png_jmpbuf(png))){
    png_destroy_write_struct(&png, &info);
    if (error)
      *error = "Can't write " + file;
    return false;
  }
  png_set_write_fn(png, &out, [](png_structp p, png_bytep bytes, png_size_t length){
    std::vector<char>* buffer = (std::vector<char>*)png_get_io_ptr(p);
    buffer->insert(buffer->end(), (const char*)bytes, (const char*)bytes + length);
  }, nullptr);
  png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);
  for (int y = 0; y < h; ++y)
    png_write_row(png, cropped.data() + (size_t)y*w*4);
  png_write_end(png, nullptr);
  png_destroy_write_struct(&png, &info);
  std::ofstream os(path, std::ios::binary | std::ios::trunc);
  os.write(out.data(), (std::streamsize)out.size());
  if (!os){
    if (error)
      *error = "Can't write " + file;
    return false;
  }
  return true;
}

}
