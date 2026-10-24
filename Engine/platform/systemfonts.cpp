#include "systemfonts.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

#include "assets.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(LINUX)
#include <fontconfig/fontconfig.h>
#endif
#if defined(ANDROID) || defined(__EMSCRIPTEN__)
#include <mutex>
#endif

// The lookups are adapted from guit3's guifont_w32.cpp (GDI), fcfont.cpp
// (fontconfig) and emfont.cpp (a folder of fonts indexed with FreeType).

using namespace CGE;

namespace{

std::string lower(std::string text){
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){return (char)std::tolower(c);});
  return text;
}

//! sorted without regard to case, each once
std::vector<std::string> sorted(std::vector<std::string> names){
  std::sort(names.begin(), names.end(), [](const std::string& a, const std::string& b){return lower(a) < lower(b);});
  names.erase(std::unique(names.begin(), names.end()), names.end());
  return names;
}

}

#ifdef WIN32

namespace{

std::wstring toWide(const std::string& utf8){
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
  std::wstring ret(len, L'\0');
  if (len > 0)
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &ret[0], len);
  return ret;
}

std::string toUtf8(const wchar_t* wide){
  int len = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
  if (len <= 1)
    return std::string();
  std::string ret(len - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide, -1, &ret[0], len, nullptr, nullptr);
  return ret;
}

int CALLBACK collectFamily(const LOGFONTW* font, const TEXTMETRICW*, DWORD, LPARAM param){
  //'@' names the vertical variants of East Asian fonts
  if (font->lfFaceName[0] != L'@')
    ((std::vector<std::string>*)param)->push_back(toUtf8(font->lfFaceName));
  return 1;
}

//! the families GDI enumerates for a name, or all of them for an empty one
std::vector<std::string> enumerate(const std::string& name){
  std::vector<std::string> families;
  LOGFONTW font = {};
  font.lfCharSet = DEFAULT_CHARSET;
  std::wstring wide = toWide(name);
  if (wide.size() >= LF_FACESIZE)
    return families;
  wcscpy_s(font.lfFaceName, wide.c_str());
  HDC dc = CreateCompatibleDC(nullptr);
  EnumFontFamiliesExW(dc, &font, collectFamily, (LPARAM)&families, 0);
  DeleteDC(dc);
  return families;
}

//! the face of a font collection with this family and the closest style; 0 if none has the family
/*! GDI gives the whole collection file but not which face it chose. */
int collectionFace(const std::vector<uint8>& data, const std::string& family, int weight, bool italic){
  FT_Library ft;
  if (FT_Init_FreeType(&ft) != 0)
    return 0;
  int best = 0;
  int bestScore = -1;
  FT_Long faces = 1;
  for (FT_Long i = 0; i < faces; ++i){
    FT_Face face;
    if (FT_New_Memory_Face(ft, data.data(), (FT_Long)data.size(), i, &face) != 0)
      break;
    faces = face->num_faces;
    if (face->family_name && lower(face->family_name) == lower(family)){
      int score = 0;
      if (((face->style_flags & FT_STYLE_FLAG_BOLD) != 0) == (weight >= 600))
        score += 1;
      if (((face->style_flags & FT_STYLE_FLAG_ITALIC) != 0) == italic)
        score += 2;
      if (score > bestScore){
        bestScore = score;
        best = (int)i;
      }
    }
    FT_Done_Face(face);
  }
  FT_Done_FreeType(ft);
  return best;
}

}

bool SystemFonts::find(const std::string& family, FontData& font, int weight, bool italic){
  //GDI knows no generic names
  std::string name = family;
  std::string generic = lower(family);
  if (generic == "sans-serif" || generic.empty())
    name = "Arial";
  else if (generic == "serif")
    name = "Times New Roman";
  else if (generic == "monospace")
    name = "Consolas";
  std::wstring wide = toWide(name);
  if (wide.size() >= LF_FACESIZE)
    return false;
  //GDI's font matching picks the file, also a substitute for a missing family
  LOGFONTW desc = {};
  desc.lfWeight = weight;
  desc.lfItalic = italic ? TRUE : FALSE;
  desc.lfCharSet = DEFAULT_CHARSET;
  desc.lfOutPrecision = OUT_TT_ONLY_PRECIS;
  wcscpy_s(desc.lfFaceName, wide.c_str());
  HFONT handle = CreateFontIndirectW(&desc);
  if (!handle)
    return false;
  HDC dc = CreateCompatibleDC(nullptr);
  HGDIOBJ old = SelectObject(dc, handle);
  wchar_t face[LF_FACESIZE] = L"";
  GetTextFaceW(dc, LF_FACESIZE, face);
  //a collection's whole file is its 'ttcf' table; table 0 would be one face
  //with offsets into the collection
  const DWORD ttcf = 0x66637474;
  DWORD table = ttcf;
  DWORD size = GetFontData(dc, table, 0, nullptr, 0);
  if (size == GDI_ERROR){
    table = 0;
    size = GetFontData(dc, table, 0, nullptr, 0);
  }
  bool found = false;
  //GDI_ERROR also for fonts without outlines, e.g. "System"
  if (size != GDI_ERROR && size > 0){
    font.data.resize(size);
    found = GetFontData(dc, table, 0, font.data.data(), size) == size;
  }
  SelectObject(dc, old);
  DeleteDC(dc);
  DeleteObject(handle);
  if (!found){
    font.data.clear();
    return false;
  }
  font.family = toUtf8(face);
  font.faceIndex = table == ttcf ? collectionFace(font.data, font.family, weight, italic) : 0;
  return true;
}

bool SystemFonts::exists(const std::string& family){
  return !family.empty() && !enumerate(family).empty();
}

std::vector<std::string> SystemFonts::getFamilies(){
  return sorted(enumerate(""));
}

#elif defined(LINUX)

namespace{

//! the families fontconfig lists for a pattern
std::vector<std::string> list(FcPattern* pattern){
  std::vector<std::string> families;
  FcObjectSet* objects = FcObjectSetBuild(FC_FAMILY, (char*)0);
  FcFontSet* set = FcFontList(nullptr, pattern, objects);
  if (set){
    for (int i = 0; i < set->nfont; ++i){
      FcChar8* name = nullptr;
      if (FcPatternGetString(set->fonts[i], FC_FAMILY, 0, &name) == FcResultMatch && name)
        families.push_back((const char*)name);
    }
    FcFontSetDestroy(set);
  }
  FcObjectSetDestroy(objects);
  return families;
}

}

bool SystemFonts::find(const std::string& family, FontData& font, int weight, bool italic){
  FcPattern* pattern = FcPatternCreate();
  FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)(family.empty() ? "sans-serif" : family.c_str()));
  FcPatternAddInteger(pattern, FC_WEIGHT, FcWeightFromOpenType(weight));
  FcPatternAddInteger(pattern, FC_SLANT, italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
  //the configured substitutes and defaults, as for other applications
  FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  FcResult result;
  FcPattern* match = FcFontMatch(nullptr, pattern, &result);
  FcPatternDestroy(pattern);
  if (!match)
    return false;
  FcChar8* file = nullptr;
  FcChar8* name = nullptr;
  int index = 0;
  bool found = false;
  if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch && file){
    found = Assets::load((const char*)file, font.data) && !font.data.empty();
    FcPatternGetInteger(match, FC_INDEX, 0, &index);
    font.faceIndex = index;
    if (FcPatternGetString(match, FC_FAMILY, 0, &name) == FcResultMatch && name)
      font.family = (const char*)name;
  }
  FcPatternDestroy(match);
  return found;
}

bool SystemFonts::exists(const std::string& family){
  if (family.empty())
    return false;
  //no substitutes: only fonts whose families contain the name
  FcPattern* pattern = FcPatternCreate();
  FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)family.c_str());
  bool found = !list(pattern).empty();
  FcPatternDestroy(pattern);
  return found;
}

std::vector<std::string> SystemFonts::getFamilies(){
  FcPattern* pattern = FcPatternCreate();
  std::vector<std::string> families = list(pattern);
  FcPatternDestroy(pattern);
  return sorted(families);
}

#else

namespace{

//! a face in one of the indexed font files
struct IndexedFace{
  std::string path;
  int faceIndex;
  std::string family;
  int weight;
  bool italic;
};

//! the folders whose fonts count as the system's
const char* const fontFolders[] = {
#ifdef ANDROID
  "/system/fonts",
#endif
  "fonts",
};

//! similar families for names that aren't installed, the closest first
/*! The Liberation fonts are metric-compatible with Arial, Times New Roman
    and Courier New, so texts keep their widths; Roboto, Noto and Droid
    are Android's own. */
struct Alias{
  const char* name;
  const char* families[5];
};
const char* const sansFamilies[] = {"Roboto", "DejaVu Sans", "Liberation Sans", "Noto Sans", nullptr};
const Alias aliases[] = {
  {"arial", {"Liberation Sans", "Roboto", "DejaVu Sans", "Noto Sans", nullptr}},
  {"helvetica", {"Liberation Sans", "Roboto", "DejaVu Sans", "Noto Sans", nullptr}},
  {"verdana", {"DejaVu Sans", "Roboto", "Liberation Sans", "Noto Sans", nullptr}},
  {"tahoma", {"DejaVu Sans", "Roboto", "Liberation Sans", "Noto Sans", nullptr}},
  {"serif", {"Noto Serif", "Liberation Serif", "DejaVu Serif", "Gelasio", nullptr}},
  {"times new roman", {"Liberation Serif", "Noto Serif", "DejaVu Serif", nullptr}},
  {"times", {"Liberation Serif", "Noto Serif", "DejaVu Serif", nullptr}},
  {"georgia", {"Gelasio", "Liberation Serif", "Noto Serif", "DejaVu Serif", nullptr}},
  {"monospace", {"Droid Sans Mono", "DejaVu Sans Mono", "Liberation Mono", "Cutive Mono", nullptr}},
  {"courier new", {"Liberation Mono", "Cutive Mono", "Droid Sans Mono", "DejaVu Sans Mono", nullptr}},
  {"courier", {"Liberation Mono", "Cutive Mono", "Droid Sans Mono", "DejaVu Sans Mono", nullptr}},
  {"consolas", {"Liberation Mono", "Droid Sans Mono", "DejaVu Sans Mono", nullptr}},
  {"comic sans ms", {"Comic Relief", "Coming Soon", nullptr}},
};

std::mutex indexMutex;
std::vector<IndexedFace> fontIndex;
bool indexed = false;

//! the faces of a font file; absolute paths are read by FreeType itself, so big system fonts aren't loaded
void indexFile(FT_Library ft, const std::string& path, std::vector<IndexedFace>& faces){
  std::vector<uint8> data;
  bool asset = !Assets::isAbsolute(path);
  if (asset && !Assets::load(path, data))
    return;
  FT_Long count = 1;
  for (FT_Long i = 0; i < count; ++i){
    FT_Face face;
    FT_Error error = asset ? FT_New_Memory_Face(ft, data.data(), (FT_Long)data.size(), i, &face)
      : FT_New_Face(ft, path.c_str(), i, &face);
    if (error != 0)
      return;
    count = face->num_faces;
    if (face->family_name && FT_IS_SCALABLE(face)){
      IndexedFace entry;
      entry.path = path;
      entry.faceIndex = (int)i;
      entry.family = face->family_name;
      TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
      if (os2 && os2->usWeightClass > 0)
        entry.weight = os2->usWeightClass;
      else
        entry.weight = (face->style_flags & FT_STYLE_FLAG_BOLD) ? 700 : 400;
      entry.italic = (face->style_flags & FT_STYLE_FLAG_ITALIC) != 0;
      faces.push_back(entry);
    }
    FT_Done_Face(face);
  }
}

//! the index, made on first use
const std::vector<IndexedFace>& getIndex(){
  std::lock_guard<std::mutex> lock(indexMutex);
  if (!indexed){
    indexed = true;
    FT_Library ft;
    if (FT_Init_FreeType(&ft) == 0){
      for (const char* folder : fontFolders){
        for (const std::string& file : Assets::listFiles(folder)){
          std::string ext = lower(file.size() > 4 ? file.substr(file.size()-4) : "");
          if (ext == ".ttf" || ext == ".otf" || ext == ".ttc")
            indexFile(ft, std::string(folder) + "/" + file, fontIndex);
        }
      }
      FT_Done_FreeType(ft);
    }
  }
  return fontIndex;
}

//! the face of a family closest to the weight and slant
const IndexedFace* findFace(const std::string& family, int weight, bool italic){
  const IndexedFace* best = nullptr;
  int bestScore = 0;
  std::string name = lower(family);
  for (const IndexedFace& face : getIndex()){
    if (lower(face.family) != name)
      continue;
    int score = std::abs(face.weight - weight) + (face.italic != italic ? 1000 : 0);
    if (!best || score < bestScore){
      best = &face;
      bestScore = score;
    }
  }
  return best;
}

}

bool SystemFonts::find(const std::string& family, FontData& font, int weight, bool italic){
  const IndexedFace* face = family.empty() ? nullptr : findFace(family, weight, italic);
  //an alias, or else a sans-serif font, or else any
  const char* const* candidates = sansFamilies;
  std::string name = lower(family);
  for (const Alias& alias : aliases){
    if (name == alias.name)
      candidates = alias.families;
  }
  for (const char* const* candidate = candidates; !face && *candidate; ++candidate)
    face = findFace(*candidate, weight, italic);
  if (!face && !getIndex().empty())
    face = findFace(getIndex().front().family, weight, italic);
  if (!face || !Assets::load(face->path, font.data) || font.data.empty())
    return false;
  font.faceIndex = face->faceIndex;
  font.family = face->family;
  return true;
}

bool SystemFonts::exists(const std::string& family){
  std::string name = lower(family);
  for (const IndexedFace& face : getIndex()){
    if (lower(face.family) == name)
      return true;
  }
  return false;
}

std::vector<std::string> SystemFonts::getFamilies(){
  std::vector<std::string> families;
  for (const IndexedFace& face : getIndex())
    families.push_back(face.family);
  return sorted(families);
}

#endif
