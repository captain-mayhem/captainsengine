#include "gamefile.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <set>

#include <image/image.h>
#include <image/loader.h>
#include <io/ZipReader.h>
#include <io/ZipWriter.h>

#include "bitmapfont.h"
#include "images.h"
#include "projectfile.h"
#include "resources.h"

//jpeglib after the engine's headers, which may bring windows.h: its INT32
//is another type there (basetsd.h), and jpeglib's own one is renamed for the
//case that basetsd.h comes later; its boolean is windows.h's (jconfig.h)
#ifdef WIN32
#undef FAR
#define INT32 jpeg_int32
#endif
extern "C"{
#include <jpeglib.h>
}
#ifdef WIN32
#undef INT32
#endif

namespace fs = std::filesystem;

namespace sd{

namespace{

fs::path toPath(const std::string& cp1252){
  std::string utf8 = toUtf8(cp1252);
  return fs::path(std::u8string(utf8.begin(), utf8.end()));
}

std::string fromPath(const fs::path& path){
  std::u8string u8 = path.u8string();
  return fromUtf8(std::string(u8.begin(), u8.end()));
}

//! a path as minizip's fopen takes it
std::string nativeName(const fs::path& path){
#if defined(WIN32)
  return path.string();
#else
  std::u8string u8 = path.u8string();
  return std::string(u8.begin(), u8.end());
#endif
}

bool readFile(const fs::path& path, std::string& data){
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return false;
  data.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  return true;
}

bool writeFile(const fs::path& path, const void* data, size_t size){
  std::error_code ec;
  if (path.has_parent_path())
    fs::create_directories(path.parent_path(), ec);
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out)
    return false;
  out.write((const char*)data, (std::streamsize)size);
  return (bool)out;
}

bool writeFile(const fs::path& path, const std::string& data){
  return writeFile(path, data.data(), data.size());
}

std::string extensionOf(const std::string& name){
  std::string file = fileNameOf(name);
  size_t dot = file.find_last_of('.');
  return dot == std::string::npos ? std::string() : file.substr(dot);
}

std::string stemOf(const std::string& name){
  std::string file = fileNameOf(name);
  size_t dot = file.find_last_of('.');
  return dot == std::string::npos ? file : file.substr(0, dot);
}

std::string lower(const std::string& s){
  return toLower(s);
}

//! a zip in memory with its entries' bytes
std::string zipOf(CGE::ZipWriter& zip){
  zip.close();
  return std::string(zip.getData().begin(), zip.getData().end());
}

//! a JPEG of an image of 1 or 3 channels (GDI+'s default quality)
std::string encodeJpeg(const uint8_t* pixels, int width, int height, int channels){
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  unsigned char* buffer = nullptr;
  size_t size = 0;
  jpeg_mem_dest(&cinfo, &buffer, &size);
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = channels;
  cinfo.in_color_space = channels == 1 ? JCS_GRAYSCALE : JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 75, TRUE);
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height){
    JSAMPROW row = (JSAMPROW)(pixels + (size_t)cinfo.next_scanline*width*channels);
    jpeg_write_scanlines(&cinfo, &row, 1);
  }
  jpeg_finish_compress(&cinfo);
  std::string out((const char*)buffer, size);
  jpeg_destroy_compress(&cinfo);
  free(buffer);
  return out;
}

bool isPng(const std::string& data){
  return data.size() > 8 && data.compare(0, 4, "\x89PNG") == 0;
}

//! fonts.dat: font.00N for each of the project's fonts
std::string writeFonts(const Project& project, std::vector<std::string>& warnings){
  CGE::ZipWriter master;
  for (size_t i = 0; i < project.settings.fonts.size(); ++i){
    int number = (int)i+1;
    BitmapFont font;
    std::string error;
    if (!renderBitmapFont(project.settings.fonts[i], font, &error)){
      warnings.push_back("Font " + std::to_string(number) + ": " + error);
      continue;
    }
    std::string archive = fontArchive(font, number);
    master.addEntry("font.00" + std::to_string(number), archive.data(), archive.size());
  }
  return zipOf(master);
}

bool copyFontDat(const GameOptions& options, const fs::path& datadir, std::vector<std::string>& warnings){
  std::string font;
  if (!readFile(toPath(options.runtimeFolder) / "font.dat", font)){
    warnings.push_back("font.dat (the game's own font) is missing next to the StoryDesigner.");
    return false;
  }
  return writeFile(datadir / "font.dat", font);
}

void progress(const GameOptions& options, const std::string& step){
  if (options.progress)
    options.progress(step);
}

//! a .dat of media files, each under its file's name
bool packMedia(const Project& project, const std::string& folder, ResourceID kind, const fs::path& out, const std::string& password,
  const GameOptions& options, std::vector<std::string>& warnings){
  CGE::ZipWriter zip(nativeName(out));
  if (!zip.isWorking())
    return false;
  const NamedList<std::string>* files = mediaFiles(project, kind);
  for (const auto& entry : *files){
    std::string name = fileNameOf(entry.value);
    std::string data;
    bool found = false;
    if (kind == ResourceID::Video && options.playableVideo){
      //videos the engine can't play are converted now; the entry keeps its
      //name, the engine tells the format by the content
      std::string file = findMediaFile(project, folder, kind, entry.key);
      if (!file.empty()){
        std::string error;
        std::string playable = options.playableVideo(file, error);
        if (playable.empty())
          warnings.push_back(error);
        else
          found = readFile(toPath(playable), data);
      }
    }
    if (!found){
      std::vector<char> bytes;
      if (!readMediaFile(project, folder, kind, entry.key, bytes)){
        warnings.push_back("Not found: " + toUtf8(entry.value));
        continue;
      }
      data.assign(bytes.begin(), bytes.end());
    }
    if (!zip.addEntry(name, data.data(), data.size(), password))
      return false;
  }
  return zip.close();
}

bool packGraphics(const Project& project, const std::string& folder, const fs::path& out, const std::string& password, std::vector<std::string>& warnings){
  CGE::ZipWriter zip(nativeName(out));
  if (!zip.isWorking())
    return false;
  for (const auto& entry : project.images){
    std::string name = fileNameOf(entry.value);
    std::vector<char> bytes;
    if (!readMediaFile(project, folder, ResourceID::Image, entry.key, bytes)){
      warnings.push_back("Not found: " + toUtf8(entry.value));
      continue;
    }
    std::string data(bytes.begin(), bytes.end());
    if (project.settings.pngToJpeg && isPng(data)){
      //a PNG as two JPEGs, its colours (.pnj) and its alpha (.pna)
      std::unique_ptr<CGE::Image> img(CGE::ImageLoader::load(&data[0], (unsigned)data.size(), CGE::ImageLoader::PNG));
      if (img && img->getNumChannels() != 4)
        img.reset(img->convertImage(4));
      if (img){
        int w = (int)img->getWidth(), h = (int)img->getHeight();
        std::vector<uint8_t> rgb((size_t)w*h*3), alpha((size_t)w*h);
        const uint8_t* src = img->getData();
        for (size_t i = 0, n = (size_t)w*h; i < n; ++i){
          rgb[i*3] = src[i*4];
          rgb[i*3+1] = src[i*4+1];
          rgb[i*3+2] = src[i*4+2];
          alpha[i] = src[i*4+3];
        }
        std::string base = stemOf(name);
        std::string pnj = encodeJpeg(rgb.data(), w, h, 3);
        std::string pna = encodeJpeg(alpha.data(), w, h, 1);
        if (!zip.addEntry(base + ".pnj", pnj.data(), pnj.size(), password) || !zip.addEntry(base + ".pna", pna.data(), pna.size(), password))
          return false;
        continue;
      }
      warnings.push_back("Can't read the PNG " + toUtf8(name) + "; it is packed as it is.");
    }
    if (!zip.addEntry(name, data.data(), data.size(), password))
      return false;
  }
  return zip.close();
}

bool writeEntries(const std::vector<std::pair<std::string, std::string>>& entries, const std::string& name, const fs::path& out){
  CGE::ZipWriter zip(nativeName(out));
  if (!zip.isWorking())
    return false;
  for (const auto& entry : entries){
    if (!zip.addEntry(name + entry.first, entry.second.data(), entry.second.size()))
      return false;
  }
  return zip.close();
}

//! unzips an archive's entries into a folder, the .zip entries into folders of their names
bool unzipRecursive(CGE::ZipReader& zip, const fs::path& outdir){
  for (const std::string& name : zip.getEntries()){
    if (name.empty() || name.back() == '/')
      continue;
    CGE::MemReader rdr = zip.openEntry(name);
    if (lower(extensionOf(name)) == ".zip"){
      fs::path dir = outdir / toPath(stemOf(name));
      std::error_code ec;
      fs::create_directories(dir, ec);
      std::string inner(rdr.getData() ? rdr.getData() : "", rdr.getSize());
      CGE::ZipReader innerZip(inner.empty() ? nullptr : &inner[0], (uint32_t)inner.size());
      if (!unzipRecursive(innerZip, dir))
        return false;
    }
    else if (!writeFile(outdir / toPath(name), rdr.getData(), rdr.getSize()))
      return false;
  }
  return true;
}

//! an archive read into memory, so that any path works on every platform; null if it can't be read
std::unique_ptr<CGE::ZipReader> openZip(const fs::path& path, std::string& data){
  if (!readFile(path, data) || data.empty())
    return nullptr;
  std::unique_ptr<CGE::ZipReader> zip(new CGE::ZipReader(&data[0], (uint32_t)data.size()));
  if (!zip->isWorking())
    return nullptr;
  return zip;
}

std::string readEntryText(CGE::ZipReader& zip, const std::string& name){
  CGE::MemReader rdr = zip.openEntry(name);
  return std::string(rdr.getData() ? rdr.getData() : "", rdr.getSize());
}

}

bool unpackRuntime(const std::string& archive, const std::string& folder, const std::string& program, std::string* error){
  std::string data;
  std::unique_ptr<CGE::ZipReader> archiveZip = openZip(toPath(archive), data);
  if (!archiveZip){
    if (error)
      *error = toUtf8(fileNameOf(archive)) + " is missing next to the StoryDesigner.";
    return false;
  }
  CGE::ZipReader& zip = *archiveZip;
  for (const std::string& name : zip.getEntries()){
    if (name.empty() || name.back() == '/')
      continue;
    std::string target = name;
    bool isProgram = lower(extensionOf(name)) == ".exe" || name == "Adventure";
    if (isProgram && !program.empty())
      target = program;
    CGE::MemReader rdr = zip.openEntry(name);
    fs::path out = toPath(folder) / toPath(target);
    if (!writeFile(out, rdr.getData(), rdr.getSize())){
      if (error)
        *error = "Can't write " + toUtf8(fromPath(out)) + ".";
      return false;
    }
#if !defined(WIN32)
    if (isProgram){
      std::error_code ec;
      fs::permissions(out, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add, ec);
    }
#endif
  }
  return true;
}

bool writeGame(Project& project, const std::string& folder, const std::string& gamePath, const GameOptions& options,
  std::vector<std::string>& warnings, std::string* error){
  auto fail = [&](const std::string& why){
    if (error)
      *error = why;
    return false;
  };
  fs::path game = toPath(gamePath);
  fs::path dir = game.parent_path();
  fs::path datadir = dir / "data";
  std::error_code ec;
  fs::create_directories(datadir, ec);
  if (!fs::is_directory(datadir, ec))
    return fail("Can't make the folder " + toUtf8(fromPath(datadir)) + ".");
  progress(options, "game.dat");
  //the entries first: a protected project gets its password there
  auto entries = writeProjectEntries(project, true);
  if (!writeEntries(entries, "game", datadir / "game.dat"))
    return fail("Can't write game.dat. Please make sure that it is a writable file.");
  std::string password = project.settings.protectGameFile ? project.zipPassword : std::string();
  progress(options, "fonts.dat");
  if (!writeFile(datadir / "fonts.dat", writeFonts(project, warnings)))
    return fail("Can't write fonts.dat.");
  copyFontDat(options, datadir, warnings);
  progress(options, "gfx.dat");
  if (!packGraphics(project, folder, datadir / "gfx.dat", password, warnings))
    return fail("Can't write gfx.dat.");
  progress(options, "sfx.dat");
  if (!packMedia(project, folder, ResourceID::Sound, datadir / "sfx.dat", password, options, warnings))
    return fail("Can't write sfx.dat.");
  progress(options, "music.dat");
  if (!packMedia(project, folder, ResourceID::Music, datadir / "music.dat", password, options, warnings))
    return fail("Can't write music.dat.");
  progress(options, "movie.dat");
  if (!packMedia(project, folder, ResourceID::Video, datadir / "movie.dat", password, options, warnings))
    return fail("Can't write movie.dat.");
  progress(options, "runtime");
  std::string runtimeError;
  std::string program = fromPath(game.filename());
  //the engine's fonts, then the Adventure and its libraries
  if (!unpackRuntime(fromPath(toPath(options.runtimeFolder) / "engine2.dat"), fromPath(dir), std::string(), &runtimeError))
    warnings.push_back("The engine's fonts aren't added: " + runtimeError);
  if (!unpackRuntime(fromPath(toPath(options.runtimeFolder) / "engine1.dat"), fromPath(dir), program, &runtimeError))
    warnings.push_back("Unable to generate the adventure runtime: " + runtimeError);
  //the loading screen, as loading<ext>
  const std::string& loading = project.settings.loadingImage;
  if (!loading.empty() && project.images.find(loading)){
    std::vector<char> bytes;
    if (readMediaFile(project, folder, ResourceID::Image, loading, bytes))
      writeFile(datadir / toPath("loading" + extensionOf(*project.images.find(loading))), bytes.data(), bytes.size());
    else
      warnings.push_back("The loading image " + toUtf8(loading) + " isn't found.");
  }
  return true;
}

std::string writeTestGame(Project& project, const std::string& folder, const std::string& dataFolder, const GameOptions& options,
  std::vector<std::string>& warnings, std::string* error){
  fs::path datadir = toPath(dataFolder);
  std::error_code ec;
  fs::create_directories(datadir, ec);
  //the media where they are found, for the game to read them
  Project copy = project;
  bool inDat = false;
  for (ResourceID kind : {ResourceID::Image, ResourceID::Sound, ResourceID::Music, ResourceID::Video}){
    for (auto& entry : *mediaFiles(copy, kind)){
      std::string file = findMediaFile(project, folder, kind, entry.key);
      if (!file.empty())
        entry.value = file;
      else
        inDat = true;
    }
  }
  fs::path projectFile;
  if (inDat){
    //a game's data: game.dat beside copies of its .dat files
    for (const char* dat : {"gfx.dat", "sfx.dat", "music.dat", "movie.dat"}){
      fs::path from = toPath(folder) / dat;
      if (fs::is_regular_file(from, ec))
        fs::copy_file(from, datadir / dat, fs::copy_options::update_existing, ec);
    }
    projectFile = datadir / "game.dat";
    if (!writeEntries(writeProjectEntries(copy, true), "game", projectFile)){
      if (error)
        *error = "Can't write " + toUtf8(fromPath(projectFile)) + ".";
      return std::string();
    }
  }
  else{
    std::string name = copy.settings.projectName.empty() ? "game" : copy.settings.projectName;
    projectFile = datadir / toPath(name + ".adv");
    if (!writeEntries(writeProjectEntries(copy, false), name, projectFile)){
      if (error)
        *error = "Can't write " + toUtf8(fromPath(projectFile)) + ".";
      return std::string();
    }
  }
  //the password the copy may have got
  project.zipPassword = copy.zipPassword;
  if (!writeFile(datadir / "fonts.dat", writeFonts(copy, warnings))){
    if (error)
      *error = "Can't write fonts.dat.";
    return std::string();
  }
  copyFontDat(options, datadir, warnings);
  return fromPath(projectFile);
}

namespace{

//! the media pool's zips of a kind: <folder>.zip for each folder with files
//! (AdvFileWriter.writeFileRecursive), its subfolders' before its own
void exportFolder(const Project& project, const std::string& folder, const PoolNode& node, ResourceID kind, CGE::ZipWriter& out, std::vector<std::string>& warnings){
  bool hasData = false;
  for (const PoolNode& child : node.children){
    if (child.id == (int)ResourceID::Folder)
      exportFolder(project, folder, child, kind, out, warnings);
    else
      hasData = true;
  }
  if (!hasData)
    return;
  CGE::ZipWriter files;
  for (const PoolNode& child : node.children){
    if (child.id == (int)ResourceID::Folder)
      continue;
    const std::string* file = mediaFiles(project, kind)->find(child.label);
    std::vector<char> bytes;
    if (file == nullptr || !readMediaFile(project, folder, kind, child.label, bytes)){
      warnings.push_back("Not found: " + toUtf8(file ? *file : child.label));
      continue;
    }
    files.addEntry(fileNameOf(*file), bytes.data(), bytes.size());
  }
  std::string data = zipOf(files);
  out.addEntry(node.label + ".zip", data.data(), data.size());
}

}

bool writeExport(Project& project, const std::string& folder, const std::string& adzPath, std::vector<std::string>& warnings, std::string* error){
  CGE::ZipWriter zip(nativeName(toPath(adzPath)));
  if (!zip.isWorking()){
    if (error)
      *error = "Failed to write export file " + toUtf8(adzPath) + ". Please make sure that it is a writable file.";
    return false;
  }
  const struct{
    const char* dat;
    ResourceID kind;
  } kinds[] = {
    {"gfx.dat", ResourceID::Image},
    {"music.dat", ResourceID::Music},
    {"sfx.dat", ResourceID::Sound},
    {"movie.dat", ResourceID::Video},
  };
  for (const auto& k : kinds){
    if (mediaFiles(project, k.kind)->empty())
      continue;
    int top = topFolder(k.kind);
    CGE::ZipWriter dat;
    if ((size_t)top < project.mediaPool.size())
      exportFolder(project, folder, project.mediaPool[top], k.kind, dat, warnings);
    std::string data = zipOf(dat);
    zip.addEntry(k.dat, data.data(), data.size());
  }
  std::string name = project.settings.projectName;
  //the project file, as a zip in the zip
  CGE::ZipWriter adv;
  //without the password: the C# StoryDesigner exports it unprotected
  for (const auto& entry : writeProjectEntries(project, false)){
    if (entry.first != ".005")
      adv.addEntry(name + entry.first, entry.second.data(), entry.second.size());
  }
  std::string advData = zipOf(adv);
  zip.addEntry(name + ".adv", advData.data(), advData.size());
  const std::string& icon = project.settings.gameIcon;
  if (!icon.empty()){
    std::string data;
    if (readFile(toPath(icon), data) || readFile(toPath(folder) / toPath(fileNameOf(icon)), data))
      zip.addEntry(fileNameOf(icon), data.data(), data.size());
    else
      warnings.push_back("The game icon " + toUtf8(icon) + " isn't found.");
  }
  if (!zip.close()){
    if (error)
      *error = "Failed to write export file " + toUtf8(adzPath) + ".";
    return false;
  }
  return true;
}

std::string defaultUnpackFolder(const std::string& adzPath){
  fs::path path = toPath(adzPath);
  return fromPath(path.parent_path() / path.stem());
}

std::string unpackAdz(const std::string& adzPath, const std::string& destFolder, std::string* error){
  auto fail = [&](const std::string& why){
    if (error)
      *error = why;
    return std::string();
  };
  std::string data;
  std::unique_ptr<CGE::ZipReader> adz = openZip(toPath(adzPath), data);
  if (!adz)
    return fail("Can't read " + toUtf8(adzPath) + ".");
  CGE::ZipReader& zip = *adz;
  fs::path dest = toPath(destFolder);
  std::error_code ec;
  fs::create_directories(dest, ec);
  std::string advName;
  for (const std::string& name : zip.getEntries()){
    std::string ext = extensionOf(name);
    if (ext == ".adv"){
      CGE::MemReader rdr = zip.openEntry(name);
      if (!writeFile(dest / toPath(name), rdr.getData(), rdr.getSize()))
        return fail("Can't write into " + toUtf8(destFolder) + ".");
      advName = fromPath(dest / toPath(name));
    }
    else if (ext == ".dat"){
      std::string inner = readEntryText(zip, name);
      CGE::ZipReader innerZip(inner.empty() ? nullptr : &inner[0], (uint32_t)inner.size());
      if (!unzipRecursive(innerZip, dest))
        return fail("Can't write into " + toUtf8(destFolder) + ".");
    }
  }
  if (advName.empty())
    return fail(toUtf8(adzPath) + " has no project file.");
  return advName;
}

namespace{

//! list.gfx: the images of an import, in a media pool folder of the import's name
void addImageList(Project& project, const std::string& dirname, const fs::path& mediaDir, const std::string& text){
  if (project.mediaPool.empty())
    newPools(project);
  PoolNode* dir = nullptr;
  for (PoolNode& node : project.mediaPool[0].children){
    if (node.label == dirname){
      dir = &node;
      break;
    }
  }
  if (dir == nullptr){
    PoolNode node;
    node.id = (int)ResourceID::Folder;
    node.label = dirname;
    project.mediaPool[0].children.push_back(node);
    dir = &project.mediaPool[0].children.back();
  }
  size_t start = 0;
  while (start < text.size()){
    size_t end = text.find('\n', start);
    std::string line = text.substr(start, end == std::string::npos ? std::string::npos : end-start);
    start = end == std::string::npos ? text.size() : end+1;
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.empty())
      continue;
    std::string key = stemOf(line);
    if (project.images.find(key))
      continue;
    project.images.add(key, fromPath(mediaDir / toPath(fileNameOf(line))));
    PoolNode image;
    image.id = (int)ResourceID::Image;
    image.label = key;
    dir->children.push_back(image);
  }
  sortPool(dir->children);
  sortPool(project.mediaPool[0].children);
}

template<typename T>
std::set<std::string> keysOf(const NamedList<T>& list){
  std::set<std::string> keys;
  for (const auto& entry : list)
    keys.insert(entry.key);
  return keys;
}

//! the names of a list's entries that aren't among keys
template<typename T>
std::vector<std::string> addedSince(const NamedList<T>& list, const std::set<std::string>& keys){
  std::vector<std::string> added;
  for (const auto& entry : list){
    if (!keys.count(entry.key))
      added.push_back(entry.value.name);
  }
  return added;
}

}

bool importCharacter(Project& project, const std::string& adcPath, const std::string& folder, std::string* error){
  std::string data;
  std::unique_ptr<CGE::ZipReader> adc = openZip(toPath(adcPath), data);
  if (!adc){
    if (error)
      *error = "Can't read " + toUtf8(adcPath) + ".";
    return false;
  }
  CGE::ZipReader& zip = *adc;
  std::string dirname = stemOf(adcPath);
  fs::path mediaDir = toPath(folder) / toPath(dirname);
  if (project.gamePool.empty())
    newPools(project);
  for (const std::string& name : zip.getEntries()){
    if (name == "char.dat"){
      std::set<std::string> before = keysOf(project.characters);
      if (!importObjects(readEntryText(zip, name), project, nullptr, error))
        return false;
      std::vector<std::string> added = addedSince(project.characters, before);
      for (const std::string& chr : added)
        addToPool(project, ResourceID::Character, chr, PoolPath());
    }
    else if (name == "gfx.chr"){
      std::string inner = readEntryText(zip, name);
      CGE::ZipReader innerZip(inner.empty() ? nullptr : &inner[0], (uint32_t)inner.size());
      std::error_code ec;
      fs::create_directories(mediaDir, ec);
      if (!unzipRecursive(innerZip, mediaDir)){
        if (error)
          *error = "Can't write into " + toUtf8(fromPath(mediaDir)) + ".";
        return false;
      }
    }
    else if (name == "list.gfx")
      addImageList(project, dirname, mediaDir, readEntryText(zip, name));
  }
  return true;
}

bool importRoom(Project& project, const std::string& adrPath, const std::string& folder, std::string* error){
  std::string data;
  std::unique_ptr<CGE::ZipReader> adr = openZip(toPath(adrPath), data);
  if (!adr){
    if (error)
      *error = "Can't read " + toUtf8(adrPath) + ".";
    return false;
  }
  CGE::ZipReader& zip = *adr;
  std::string dirname = stemOf(adrPath);
  fs::path mediaDir = toPath(folder) / toPath(dirname);
  if (project.gamePool.empty())
    newPools(project);
  for (const std::string& name : zip.getEntries()){
    if (name == "room.dat"){
      //the room's objects in a folder of the room's name
      PoolNode objects;
      objects.id = (int)ResourceID::Folder;
      objects.label = dirname;
      std::set<std::string> before = keysOf(project.rooms);
      std::vector<std::string> created;
      if (!importObjects(readEntryText(zip, name), project, &created, error))
        return false;
      for (const std::string& obj : created){
        PoolNode node;
        node.id = (int)ResourceID::Object;
        node.label = obj;
        objects.children.push_back(node);
      }
      sortPool(objects.children);
      project.gamePool[3].children.push_back(objects);
      sortPool(project.gamePool[3].children);
      for (const std::string& room : addedSince(project.rooms, before))
        addToPool(project, ResourceID::Room, room, PoolPath());
    }
    else if (name == "gfx.roo"){
      std::string inner = readEntryText(zip, name);
      CGE::ZipReader innerZip(inner.empty() ? nullptr : &inner[0], (uint32_t)inner.size());
      std::error_code ec;
      fs::create_directories(mediaDir, ec);
      if (!unzipRecursive(innerZip, mediaDir)){
        if (error)
          *error = "Can't write into " + toUtf8(fromPath(mediaDir)) + ".";
        return false;
      }
    }
    else if (name == "script.dat"){
      if (!importScripts(readEntryText(zip, name), project, error))
        return false;
    }
    else if (name == "list.gfx")
      addImageList(project, dirname, mediaDir, readEntryText(zip, name));
  }
  return true;
}

}
