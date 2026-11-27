#include "projectfile.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <stdexcept>

#include <io/MemReader.h>
#include <io/ZipReader.h>
#include <io/ZipWriter.h>

// The C# StoryDesigner's AdvFileReader and AdvFileWriter, with its quirks: a
// file it read and saved again is the same here, byte for byte (the tests
// compare with files it wrote). Its reader threw on malformed files; here that
// is an exception that readProject turns into an error.

namespace sd{

namespace{

const int kStatesMax = 10;
const int kCharStatesMax = 36;
const int kFramesMax = 25;
const int kFrames2Max = 30;
const int kPartsMax = 2;
const int kFxShapesMax = 3;
const char* const kHeader = "Point&Click Project File. DO NOT MODIFY!!";
const char* const kWriteHeader = "3.6 Point&Click Project File. DO NOT MODIFY!!";
//! the character that stands for ';' in frame scripts
const char kSemicolonStandIn = '\xaf';

struct FormatError : std::runtime_error{
  explicit FormatError(const std::string& what) : std::runtime_error(what) {}
};

//! .NET's StreamReader.ReadLine: lines end at \r, \n or \r\n
class LineReader{
public:
  explicit LineReader(const std::string& text) : text_(text) {}
  bool endOfStream() const {return pos_ >= text_.size();}
  //! the next line, empty after the end
  std::string readLine(){
    size_t start = pos_;
    while (pos_ < text_.size() && text_[pos_] != '\r' && text_[pos_] != '\n')
      ++pos_;
    std::string line = text_.substr(start, pos_-start);
    if (pos_ < text_.size()){
      if (text_[pos_] == '\r' && pos_+1 < text_.size() && text_[pos_+1] == '\n')
        ++pos_;
      ++pos_;
    }
    return line;
  }
private:
  const std::string& text_;
  size_t pos_ = 0;
};

//! .NET's StreamWriter with Windows' line ends
class TextWriter{
public:
  TextWriter& operator<<(const std::string& s) {out_ += s; return *this;}
  TextWriter& operator<<(const char* s) {out_ += s; return *this;}
  TextWriter& operator<<(char c) {out_ += c; return *this;}
  TextWriter& operator<<(int i) {out_ += std::to_string(i); return *this;}
  TextWriter& operator<<(uint32_t i) {out_ += std::to_string(i); return *this;}
  TextWriter& operator<<(bool) = delete;
  void line() {out_ += "\r\n";}
  template<typename T>
  void line(const T& value) {*this << value; line();}
  const std::string& text() const {return out_;}
private:
  std::string out_;
};

int csBool(bool b) {return b ? -1 : 0;}

//! .NET's Substring: throws when out of range
std::string sub(const std::string& s, size_t start){
  if (start > s.size())
    throw FormatError("line too short: " + s);
  return s.substr(start);
}

std::string sub(const std::string& s, size_t start, size_t length){
  if (start + length > s.size())
    throw FormatError("line too short: " + s);
  return s.substr(start, length);
}

std::vector<std::string> split(const std::string& s, char sep){
  std::vector<std::string> parts;
  size_t start = 0;
  for (;;){
    size_t pos = s.find(sep, start);
    if (pos == std::string::npos){
      parts.push_back(s.substr(start));
      return parts;
    }
    parts.push_back(s.substr(start, pos-start));
    start = pos+1;
  }
}

std::string replaceAll(std::string s, char from, char to){
  for (char& c : s){
    if (c == from)
      c = to;
  }
  return s;
}

//! .NET's Convert.ToInt32 of a string (white space around allowed)
long long parseInteger(const std::string& s){
  const char* str = s.c_str();
  char* end = nullptr;
  long long val = strtoll(str, &end, 10);
  if (end == str)
    throw FormatError("not a number: " + s);
  while (*end == ' ' || *end == '\t')
    ++end;
  if (*end != '\0')
    throw FormatError("not a number: " + s);
  return val;
}

int toInt(const std::string& s) {return (int)parseInteger(s);}
uint32_t toUInt(const std::string& s) {return (uint32_t)parseInteger(s);}

std::string at(const std::vector<std::string>& parts, size_t idx){
  if (idx >= parts.size())
    throw FormatError("too few values");
  return parts[idx];
}

//! .NET's Single.Parse with a decimal comma, else a point
float parseFloat(const std::string& s){
  std::string str = replaceAll(s, ',', '.');
  const char* cstr = str.c_str();
  char* end = nullptr;
  float val = strtof(cstr, &end);
  if (end == cstr)
    throw FormatError("not a number: " + s);
  return val;
}

//! .NET's {0:0.##############} of a Single, with a decimal comma
std::string formatFloat(float f){
  char buf[64];
  //a Single has seven significant digits
  snprintf(buf, sizeof(buf), "%.7g", f);
  double d = strtod(buf, nullptr);
  snprintf(buf, sizeof(buf), "%.14f", d);
  std::string ret = buf;
  size_t dot = ret.find('.');
  if (dot != std::string::npos){
    while (ret.back() == '0')
      ret.pop_back();
    if (ret.back() == '.')
      ret.pop_back();
    else
      ret[dot] = ',';
  }
  if (ret == "-0")
    ret = "0";
  return ret;
}

std::string twoDigits(int val){
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d", val);
  return buf;
}

bool versionAtLeast(int major, int minor, int wantMajor, int wantMinor){
  return major > wantMajor || (major == wantMajor && minor >= wantMinor);
}

//! the version from a header line, false if it isn't one
bool readHeader(const std::string& line, int& major, int& minor){
  if (line.size() < 4 || line.substr(4) != kHeader)
    return false;
  major = line[0] - '0';
  minor = line[2] - '0';
  if (major < 0 || major > 9 || minor < 0 || minor > 9)
    throw FormatError("no version: " + line);
  return true;
}

class Reader{
public:
  explicit Reader(Project& project) : adv_(project) {}

  bool readSettings(const std::string& text){
    LineReader rdr(text);
    std::string str = rdr.readLine();
    int verMajor = 0, verMinor = 0;
    if (!readHeader(str, verMajor, verMinor))
      return false;
    verMajor_ = verMajor;
    verMinor_ = verMinor;
    ProjectSettings& s = adv_.settings;
    str = rdr.readLine();
    if (verMajor > 1)
      s.directory = rdr.readLine();
    else
      s.directory = "";
    str = rdr.readLine();
    if (str == "Resolution X : 640")
      s.resolution = Vec2i{640, 480};
    else if (str == "Resolution X : 800")
      s.resolution = Vec2i{800, 600};
    else if (str == "Resolution X : 320")
      s.resolution = Vec2i{320, 240};
    else
      throw FormatError("Resolution is invalid");
    std::string font = rdr.readLine();
    while (sub(font, 0, 11) != "GameFont : "){
      std::vector<std::string> parts = split(font, ';');
      FontInfo info;
      info.name = at(parts, 0);
      info.bold = toInt(at(parts, 1)) == -1;
      info.italic = toInt(at(parts, 2)) == -1;
      info.size = toInt(at(parts, 3));
      info.outline = toInt(at(parts, 4));
      info.charset = verMajor > 1 ? toInt(at(parts, 5)) : 0;
      if (verMajor > 2 || (verMajor == 2 && verMinor > 0)){
        info.shadow = toInt(at(parts, 6));
        info.fill = toInt(at(parts, 7));
        info.fading = toInt(at(parts, 8));
      }
      if (versionAtLeast(verMajor, verMinor, 3, 5))
        info.spacing = toInt(at(parts, 9));
      s.fonts.push_back(info);
      font = rdr.readLine();
    }
    //GameFont2 to 6 and the game icon
    for (int i = 0; i < 6; ++i)
      str = rdr.readLine();
    if (verMajor > 1){
      s.gameIcon = sub(str, 11);
      str = rdr.readLine();
      if (versionAtLeast(verMajor, verMinor, 3, 5)){
        std::vector<std::string> data = split(str, ';');
        s.loadingImage = at(data, 0);
        s.settingsPicture = at(data, 1);
      }
      else
        s.loadingImage = str;
      str = rdr.readLine();
      s.tsUseBgImage = str == "-1";
      s.tsBackground = rdr.readLine();
    }
    else{
      s.gameIcon = "";
      s.loadingImage = "";
      s.tsUseBgImage = false;
      s.tsBackground = "";
    }
    str = rdr.readLine();
    if (sub(str, 0, 14) != "Startskript : ")
      throw FormatError("expected Startskript : ");
    s.startScript = sub(str, 14);
    s.mainScript = rdr.readLine();
    s.anywhereRoom = rdr.readLine();
    str = rdr.readLine();
    s.screenChange = (ScreenChange)toInt(sub(str, 15));
    str = sub(rdr.readLine(), 9);
    auto flag = [&](size_t idx){
      if (idx >= str.size())
        throw FormatError("too few checks");
      return str[idx] == '1';
    };
    s.textOnOff = flag(0);
    s.drawDraggedItemIcons = flag(1);
    s.actionText = flag(2);
    s.showTaskbar = flag(3);
    if (verMajor > 2 || (verMajor == 2 && verMinor > 0)){
      s.notAntialiased = flag(4);
      s.taskbarFromTop = flag(5);
    }
    else{
      s.notAntialiased = false;
      s.taskbarFromTop = false;
    }
    if (verMajor >= 3){
      s.groupItems = flag(6);
      s.protectGameFile = flag(7);
    }
    else{
      s.groupItems = false;
      s.protectGameFile = false;
    }
    s.is16to9 = versionAtLeast(verMajor, verMinor, 3, 5) ? flag(8) : false;
    if (verMajor > 2 || (verMajor == 2 && verMinor > 0))
      s.actionTextHeight = toInt(rdr.readLine());
    else
      s.actionTextHeight = 0;
    str = rdr.readLine();
    s.customMenu = str == "-1";
    s.customMenuRoom = rdr.readLine();
    s.taskHeight = toInt(sub(rdr.readLine(), 13));
    s.taskRoom = sub(rdr.readLine(), 11);
    s.taskPopup = toInt(sub(rdr.readLine(), 12));
    str = rdr.readLine();
    std::string addBools = sub(str, 8, 2);
    s.silentDelete = addBools[0] == '1';
    s.infoLine = addBools[1] == '1';
    if (verMajor > 1){
      s.taskHideCompletely = sub(str, 11, 4) == "hide";
      s.rightClick = toInt(sub(str, 16));
    }
    else{
      s.taskHideCompletely = false;
      s.rightClick = 0;
    }
    if (verMajor > 2 || (verMajor == 2 && verMinor > 0)){
      s.useMouseWheel = rdr.readLine() == "-1";
      s.menuFading = toInt(rdr.readLine());
      s.textSceneFading = toInt(rdr.readLine());
      s.anywhereTransparency = toInt(rdr.readLine());
    }
    else{
      s.useMouseWheel = false;
      s.menuFading = 0;
      s.textSceneFading = 0;
      s.anywhereTransparency = 0;
    }
    s.targaColor = toUInt(sub(rdr.readLine(), 13));
    s.borderColor = toUInt(sub(rdr.readLine(), 14));
    s.backgroundColor = toUInt(sub(rdr.readLine(), 18));
    s.textColor = toUInt(sub(rdr.readLine(), 12));
    std::vector<std::string> colors = split(sub(rdr.readLine(), 15), ';');
    s.offspeechColor = toUInt(at(colors, 0));
    s.infotextColor = verMajor > 1 ? toUInt(at(colors, 1)) : 16777215;
    s.tsStyle = toInt(rdr.readLine()) - 1;
    s.tsBorderStyle = toInt(rdr.readLine()) - 1;
    s.tsBorderColor = toUInt(rdr.readLine());
    s.tsAreaColor = toUInt(rdr.readLine());
    s.tsSelectionColor = toUInt(rdr.readLine());
    s.tsTextColor = toUInt(rdr.readLine());
    if (verMajor >= 3 || (verMajor >= 2 && verMinor >= 7)){
      s.tsUseSymbols = rdr.readLine() == "-1";
      s.muteMusicWhenSpeech = rdr.readLine() == "-1";
      s.coinActivated = rdr.readLine() == "-1";
      s.coinAutoPopup = rdr.readLine() == "-1";
      s.coinRoom = rdr.readLine();
      s.coinFading = toInt(rdr.readLine());
      s.coinCenter.y = toInt(rdr.readLine());
      s.coinCenter.x = toInt(rdr.readLine());
    }
    if (versionAtLeast(verMajor, verMinor, 3, 5)){
      for (int i = 0; i < 25; ++i){
        std::vector<std::string> header = split(rdr.readLine(), ';');
        DSPEffect effect;
        effect.name = at(header, 2);
        effect.type = toInt(at(header, 3));
        for (int line = 0; line < 6; ++line){
          std::vector<std::string> vals = split(rdr.readLine(), ';');
          for (int num = 0; num < 6; ++num)
            effect.param[line][num] = toInt(at(vals, num));
        }
        if (effect.type > 0)
          s.dspEffects.push_back(effect);
      }
    }
    s.linkText = sub(rdr.readLine(), 11);
    s.giveLink = sub(rdr.readLine(), 11);
    s.walkText = sub(rdr.readLine(), 11);
    str = rdr.readLine();
    if (str == "Booleans :"){
      s.booleans.clear();
      str = rdr.readLine();
      while (str != "Commands :"){
        if (str.empty() && rdr.endOfStream())
          throw FormatError("no commands");
        std::string val = sub(str, str.size()-1);
        std::string name = val == "1" ? sub(str, 0, str.size()-2) : sub(str, 0, str.size()-1);
        s.booleans.push_back(std::make_pair(name, val == "1"));
        str = rdr.readLine();
      }
    }
    if (str == "Commands :"){
      s.commands.clear();
      str = rdr.readLine();
      while (str != "Mediapool :"){
        if (str.empty() && rdr.endOfStream())
          throw FormatError("no media pool");
        std::string prettyName = rdr.readLine();
        s.commands.push_back(std::make_pair(str, prettyName));
        str = rdr.readLine();
      }
    }
    str = rdr.readLine();
    adv_.mediaPool.clear();
    readPool(rdr, str, "Gamepool :", adv_.mediaPool);
    str = rdr.readLine();
    adv_.gamePool.clear();
    readPool(rdr, str, "Images :", adv_.gamePool);
    if (str == "Images :"){
      str = rdr.readLine();
      while (str != "Sounds :"){
        if (rdr.endOfStream())
          throw FormatError("no sounds");
        std::string filename = rdr.readLine();
        adv_.images.set(str, filename);
        str = rdr.readLine();
      }
    }
    if (str == "Sounds :"){
      str = rdr.readLine();
      while (str != "Music :"){
        if (rdr.endOfStream())
          throw FormatError("no music");
        std::string filename = rdr.readLine();
        adv_.sounds.set(str, filename);
        str = rdr.readLine();
      }
    }
    if (str == "Music :"){
      str = rdr.readLine();
      while (str != "Movie :"){
        if (rdr.endOfStream())
          throw FormatError("no movies");
        std::string filename = rdr.readLine();
        adv_.music.set(str, filename);
        str = rdr.readLine();
      }
    }
    if (str == "Movie :"){
      str = rdr.readLine();
      while (!rdr.endOfStream()){
        std::string filename = rdr.readLine();
        adv_.videos.set(str, filename);
        str = rdr.readLine();
      }
    }
    return true;
  }

  //! the pool's lines up to the line end; str is the first, then the end line
  void readPool(LineReader& rdr, std::string& str, const char* end, std::vector<PoolNode>& pool){
    //the nodes from the top to the last one added
    std::vector<PoolNode*> path;
    while (str != end){
      if (str.empty() && rdr.endOfStream())
        throw FormatError(std::string("expected ") + end);
      size_t level = 0;
      while (level < str.size() && str[level] == '"')
        ++level;
      PoolNode node;
      node.id = toInt(sub(str, level, 2));
      node.label = sub(str, level+2);
      //a deeper level than the next one is a child of the last node
      while (path.size() > level)
        path.pop_back();
      std::vector<PoolNode>& siblings = path.empty() ? pool : path.back()->children;
      siblings.push_back(node);
      path.push_back(&siblings.back());
      str = rdr.readLine();
    }
  }

  bool readObjects(const std::string& text){
    LineReader rdr(text);
    std::string str = rdr.readLine();
    int localMajor, localMinor;
    if (!readHeader(str, localMajor, localMinor))
      return false;
    str = rdr.readLine();
    adv_.cursor.clear();
    for (int state = 0; state < kStatesMax; ++state){
      CursorState cs;
      for (int frames = 0; frames < kFramesMax; ++frames){
        str = rdr.readLine();
        if (!str.empty())
          cs.frames.push_back(str);
      }
      cs.command = toInt(rdr.readLine());
      cs.fpsDivider = toInt(rdr.readLine());
      cs.highlight.x = toInt(rdr.readLine());
      cs.highlight.y = toInt(rdr.readLine());
      if (versionAtLeast(verMajor_, verMinor_, 3, 6)){
        cs.itemOffset.x = toInt(rdr.readLine());
        cs.itemOffset.y = toInt(rdr.readLine());
      }
      else
        cs.itemOffset = Vec2i{16, 16};
      adv_.cursor.push_back(cs);
    }
    return readObjectsLoop(rdr);
  }

  bool readObjectsLoop(LineReader& rdr){
    int verMajor = verMajor_, verMinor = verMinor_;
    Room* lastRoom = nullptr;
    while (!rdr.endOfStream()){
      std::string str = rdr.readLine();
      if (rdr.endOfStream())
        return true;
      if (str.size() < 2)
        continue;
      std::string rest = str.substr(2);
      size_t idx = rest.find(' ');
      if (idx == std::string::npos)
        throw FormatError("expected a name: " + str);
      std::string type = rest.substr(0, idx);
      std::string name = rest.substr(idx+1);
      if (type == "Item"){
        Item it;
        it.name = name;
        int numStates = kStatesMax;
        bool oldFormat = verMajor == 2 || (verMajor == 3 && verMinor == 0);
        if (oldFormat)
          numStates = 1;
        for (int state = 0; state < numStates; ++state){
          ItemState ist;
          for (int frames = 0; frames < kFramesMax; ++frames){
            str = rdr.readLine();
            if (str.empty())
              continue;
            if (!oldFormat){
              std::vector<std::string> parts = split(str, ';');
              ist.frames.push_back(parts[0]);
              ist.scripts.push_back(parts.size() > 1 ? replaceAll(parts[1], kSemicolonStandIn, ';') : "");
            }
            else{
              ist.frames.push_back(str);
              ist.scripts.push_back("");
            }
          }
          ist.fpsDivider = toInt(rdr.readLine());
          it.states.push_back(ist);
        }
        if (oldFormat){
          for (int i = 1; i < kStatesMax; ++i)
            it.states.push_back(ItemState());
        }
        adv_.items.set(it.name, it);
      }
      else if (type == "Object"){
        Object* obj = adv_.objects.find(name);
        if (!obj){
          Object newobj;
          newobj.name = name;
          adv_.objects.add(name, newobj);
          obj = adv_.objects.find(name);
        }
        obj->size.x = toInt(rdr.readLine());
        obj->size.y = toInt(rdr.readLine());
        obj->lighten = toInt(rdr.readLine()) != 0;
        for (int state = 0; state < kStatesMax; ++state){
          ObjectState os;
          os.fpsDivider = readExtendedFrames(rdr, os.frames);
          obj->states.push_back(os);
        }
      }
      else if (type == "Character"){
        Character chr;
        chr.name = name;
        chr.textColor = toUInt(rdr.readLine());
        chr.walkSpeed = toInt(rdr.readLine());
        if (verMajor > 0){
          chr.noZoom = toInt(rdr.readLine()) != 0;
          if (verMajor > 3 || (verMajor == 3 && verMinor > 0))
            chr.realLeftAnimations = toInt(rdr.readLine()) != 0;
          chr.memoryResistant = toInt(rdr.readLine()) != 0;
          chr.ghost = toInt(rdr.readLine()) != 0;
          chr.walkSound = rdr.readLine();
        }
        if (verMajor >= 3){
          std::vector<std::string> names = split(rdr.readLine(), ';');
          for (size_t i = 0; i+1 < names.size(); ++i)
            chr.extraStateNames.push_back(names[i]);
        }
        else
          chr.extraStateNames.assign(20, "");
        if (verMajor > 0){
          chr.font = toInt(rdr.readLine());
          chr.zoom = toInt(rdr.readLine());
        }
        else
          chr.zoom = 100;
        for (int state = 0; state < kCharStatesMax; ++state){
          CharacterState cs;
          cs.size.x = toInt(rdr.readLine());
          cs.size.y = toInt(rdr.readLine());
          cs.basepoint.x = toInt(rdr.readLine());
          cs.basepoint.y = toInt(rdr.readLine());
          cs.fpsDivider = readExtendedFrames(rdr, cs.frames);
          chr.states.push_back(cs);
        }
        adv_.characters.set(chr.name, chr);
      }
      else if (type == "Rcharacter"){
        CharacterInstance inst;
        inst.character = rdr.readLine();
        if (!adv_.characters.find(inst.character))
          throw FormatError("No character for character instance found");
        inst.name = name;
        inst.room = rdr.readLine();
        inst.position.x = toInt(rdr.readLine());
        inst.position.y = toInt(rdr.readLine());
        inst.lookDir = toInt(rdr.readLine());
        inst.unmovable = toInt(rdr.readLine()) == 0;
        inst.locked = toInt(rdr.readLine()) != 0;
        std::vector<CharacterInstance>* list = adv_.characterInstances.find(inst.room);
        if (!list){
          adv_.characterInstances.add(inst.room, std::vector<CharacterInstance>());
          list = adv_.characterInstances.find(inst.room);
        }
        list->push_back(inst);
      }
      else if (type == "Room"){
        Room room;
        room.name = name;
        room.size.x = toInt(rdr.readLine());
        room.size.y = toInt(rdr.readLine());
        room.scrollOffset.x = toInt(rdr.readLine());
        room.scrollOffset.y = toInt(rdr.readLine());
        room.depthmap.x = toInt(rdr.readLine());
        room.depthmap.y = toInt(rdr.readLine());
        room.zoom = toInt(rdr.readLine());
        room.background = rdr.readLine();
        room.parallaxBackground = rdr.readLine();
        room.doubleWalkmap = verMajor >= 3 ? toInt(rdr.readLine()) != 0 : false;
        if (versionAtLeast(verMajor, verMinor, 3, 5)){
          std::vector<std::string> col = split(rdr.readLine(), ';');
          for (int i = 0; i < 3; ++i)
            room.lighting[i] = (uint8_t)toInt(at(col, i));
        }
        if (verMajor >= 3 || (verMajor == 2 && verMinor >= 8)){
          for (int i = 0; i < kFxShapesMax; ++i){
            FxShape shape;
            std::vector<std::string> parts = split(rdr.readLine(), ';');
            shape.active = toInt(at(parts, 0)) != 0;
            shape.dependingOnRoomPosition = toInt(at(parts, 1)) != 0;
            shape.effect = toInt(rdr.readLine());
            shape.room = rdr.readLine();
            shape.depth = toInt(rdr.readLine());
            shape.mirrorOffset.x = toInt(rdr.readLine());
            shape.mirrorOffset.y = toInt(rdr.readLine());
            shape.strength = toInt(rdr.readLine());
            parts = split(rdr.readLine(), ';');
            for (int pos = 0; pos < 4; ++pos){
              shape.positions[pos].x = toInt(at(parts, 2*pos));
              shape.positions[pos].y = toInt(at(parts, 2*pos+1));
            }
            room.fxShapes.push_back(shape);
          }
        }
        else{
          for (int i = 0; i < kFxShapesMax; ++i)
            room.fxShapes.push_back(FxShape::makeDefault(i));
        }
        room.hasInventory = toInt(rdr.readLine()) != 0;
        std::vector<std::string> inventory = split(rdr.readLine(), ';');
        room.invPos.x = toInt(at(inventory, 0));
        room.invPos.y = toInt(at(inventory, 1));
        room.invSize.x = toInt(at(inventory, 2));
        room.invSize.y = toInt(at(inventory, 3));
        if (verMajor < 3)
          room.hasInventory = room.invSize.x != 0 && room.invSize.y != 0;
        room.invScale.x = parseFloat(at(inventory, 4));
        room.invScale.y = parseFloat(at(inventory, 5));
        room.invSpacing = versionAtLeast(verMajor, verMinor, 3, 5) ? toInt(at(inventory, 6)) : 10;
        //the walkmap, older versions had half the fields
        str = rdr.readLine();
        int walkmapX = 32;
        int walkGridSize = adv_.settings.resolution.x / walkmapX;
        int walkmapY = adv_.settings.resolution.y / walkGridSize;
        walkmapX *= 3;
        walkmapY *= 2;
        int walkmapXOut = walkmapX*2;
        int walkmapYOut = walkmapY*2;
        if (verMajor >= 3){
          walkmapX *= 2;
          walkmapY *= 2;
        }
        room.walkmap.assign(walkmapXOut, std::vector<WalkMapEntry>(walkmapYOut));
        if (str.size() < (size_t)(2*walkmapX*walkmapY))
          throw FormatError("walkmap too short in room " + room.name);
        for (int i = 0; i < walkmapX*walkmapY; ++i){
          int x = i / walkmapY;
          int y = i % walkmapY;
          room.walkmap[x][y].isFree = str[2*i] != '1';
          room.walkmap[x][y].hasScript = str[2*i+1] == '1';
        }
        adv_.rooms.set(room.name, room);
        lastRoom = adv_.rooms.find(room.name);
      }
      else if (type == "Roomobject"){
        std::string objname = rdr.readLine();
        if (!adv_.objects.find(objname)){
          //as in a room's import
          Object newobj;
          newobj.name = objname;
          newobj.states.resize(kStatesMax);
          adv_.objects.add(objname, newobj);
          if (createdObjects_)
            createdObjects_->push_back(objname);
        }
        ObjectInstance inst;
        inst.object = objname;
        inst.name = name;
        inst.position.x = toInt(rdr.readLine());
        inst.position.y = toInt(rdr.readLine());
        inst.state = toInt(rdr.readLine());
        inst.layer = toInt(rdr.readLine());
        inst.depth = toInt(rdr.readLine())*2;
        if (versionAtLeast(verMajor, verMinor, 3, 5))
          inst.depth = toInt(rdr.readLine());
        inst.locked = toInt(rdr.readLine()) != 0;
        if (!lastRoom)
          throw FormatError("room object outside a room");
        lastRoom->objects.push_back(inst);
      }
    }
    return true;
  }

  int readExtendedFrames(LineReader& rdr, std::vector<ExtendedFrame>& frames){
    int realFrames = 0;
    for (int frms = 0; frms < kFrames2Max; ++frms){
      ExtendedFrame frm;
      bool set[kPartsMax] = {false, false};
      for (int parts = 0; parts < kPartsMax; ++parts){
        std::string str = rdr.readLine();
        if (!str.empty()){
          set[parts] = true;
          frm.names.push_back(str);
        }
      }
      std::vector<std::string> parts = split(rdr.readLine(), ';');
      for (int i = 0; i < kPartsMax; ++i){
        if (!set[i])
          continue;
        frm.offsets.push_back(Vec2i{toInt(at(parts, 2*i)), toInt(at(parts, 2*i+1))});
      }
      if (set[0] || set[1])
        realFrames = frms+1;
      if (parts.size() > kPartsMax*2)
        frm.script = replaceAll(parts[kPartsMax*2], kSemicolonStandIn, ';');
      frames.push_back(frm);
    }
    frames.resize(realFrames);
    return toInt(rdr.readLine());
  }

  bool readScripts(const std::string& text){
    LineReader rdr(text);
    std::string str = rdr.readLine();
    int verMajor, verMinor;
    if (!readHeader(str, verMajor, verMinor))
      return false;
    return readScriptsLoop(rdr);
  }

  bool readScriptsLoop(LineReader& rdr){
    std::string str = rdr.readLine();
    Script* last = nullptr;
    while (!rdr.endOfStream()){
      str = rdr.readLine();
      if (str.size() >= 2 && str.compare(0, 2, "//") == 0){
        std::string rest = str.substr(2);
        size_t idx = rest.find(' ');
        if (idx == std::string::npos)
          throw FormatError("expected a script name: " + str);
        std::string type = rest.substr(0, idx);
        Script scr;
        if (type == "Cutscene")
          scr.type = Script::Cutscene;
        else if (type == "Item")
          scr.type = Script::Item;
        else if (type == "Character")
          scr.type = Script::Character;
        else if (type == "Room")
          scr.type = Script::Room;
        else if (type == "Object")
          scr.type = Script::Object;
        else if (type == "Walkmap")
          scr.type = Script::Walkmap;
        else
          return false;
        scr.name = rest.substr(idx+1);
        adv_.scripts[scr.type].set(scr.name, scr);
        last = adv_.scripts[scr.type].find(scr.name);
      }
      else if (last){
        if (!last->text)
          last->text = str;
        else
          *last->text += "\n" + str;
      }
    }
    return true;
  }

  bool readPassword(const std::string& text){
    LineReader rdr(text);
    std::string str = rdr.readLine();
    std::string pwd;
    for (int i = 0; i < 20; ++i){
      int val = toInt(sub(str, i*3, 3));
      if (i >= 15)
        val /= 3;
      else if (i >= 10)
        val /= 6;
      else if (i >= 5)
        val /= 4;
      else
        val /= 5;
      pwd += (char)val;
    }
    adv_.zipPassword = pwd;
    return true;
  }

  bool readLanguages(const std::string& text){
    LineReader rdr(text);
    std::string language;
    bool haveLanguage = false;
    int section = Language::NumSections;
    while (!rdr.endOfStream()){
      std::string str = rdr.readLine();
      if (str.compare(0, 3, "*/*") == 0){
        std::vector<std::string> parts = split(str, ';');
        language = parts[0].substr(3);
        haveLanguage = true;
        std::string name = at(parts, 1);
        if (name == "speech")
          section = Language::Speech;
        else if (name == "voicespeech")
          section = Language::SpeechSounds;
        else if (name == "offspeech")
          section = Language::Offspeech;
        else if (name == "voiceoffspeech")
          section = Language::OffspeechSounds;
        else if (name == "showinfo")
          section = Language::Showinfo;
        else if (name == "textout")
          section = Language::Textout;
        else if (name == "setstring")
          section = Language::Setstring;
        else if (name == "row")
          section = Language::Textscenes;
        else if (name == "kommandos")
          section = Language::Commands;
        continue;
      }
      if (!haveLanguage || section >= Language::NumSections)
        throw FormatError("text outside a language section");
      Language* lang = adv_.languages.find(language);
      if (!lang){
        Language newlang;
        newlang.name = language;
        adv_.languages.add(language, newlang);
        lang = adv_.languages.find(language);
      }
      lang->sections[section].push_back(str);
    }
    if (adv_.languages.empty()){
      Language origin;
      origin.name = "origin";
      adv_.languages.add("origin", origin);
    }
    return true;
  }

  bool readAdvancedSettings(const std::string& text){
    LineReader rdr(text);
    std::string str = rdr.readLine();
    int verMajor, verMinor;
    if (!readHeader(str, verMajor, verMinor))
      return false;
    verMajor_ = verMajor;
    verMinor_ = verMinor;
    str = rdr.readLine();
    str = rdr.readLine();
    adv_.settings.pngToJpeg = str == "-1";
    if (verMajor == 3 && verMinor < 5)
      return true;
    str = rdr.readLine();
    if (str == "PCDK" || str == "0")
      adv_.settings.scriptingLanguage = ScriptLang::PCDK;
    else if (str == "LUA" || str == "1")
      adv_.settings.scriptingLanguage = ScriptLang::Lua;
    else
      throw FormatError("unknown scripting language " + str);
    return true;
  }

  int verMajor_ = 0;
  int verMinor_ = 0;
  //! the objects a room's import created for its placed objects
  std::vector<std::string>* createdObjects_ = nullptr;
private:
  Project& adv_;
};

std::string extension(const std::string& name){
  size_t dot = name.find_last_of('.');
  size_t slash = name.find_last_of("/\\");
  if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
    return "";
  return name.substr(dot);
}

std::string baseName(const std::string& path){
  size_t slash = path.find_last_of("/\\");
  std::string name = slash == std::string::npos ? path : path.substr(slash+1);
  size_t dot = name.find_last_of('.');
  return dot == std::string::npos ? name : name.substr(0, dot);
}

std::string readEntry(CGE::ZipReader& zip, const std::string& name){
  CGE::MemReader rdr = zip.openEntry(name);
  std::string ret(rdr.getSize(), '\0');
  if (!ret.empty())
    rdr.readBytes((unsigned char*)&ret[0], (unsigned)ret.size());
  return ret;
}

bool readZip(CGE::ZipReader& zip, const std::string& filename, Project& project, std::string* error){
  if (!zip.isWorking()){
    if (error)
      *error = "cannot open " + filename;
    return false;
  }
  project = Project();
  project.cursor.clear();
  project.settings.projectName = extension(filename) == ".dat" ? "game" : baseName(filename);
  Reader reader(project);
  try{
    //by the extension, in the archive's order as the C# reader did
    for (const std::string& name : zip.getEntries()){
      std::string ext = extension(name);
      bool ok = true;
      if (ext == ".001")
        ok = reader.readSettings(readEntry(zip, name));
      else if (ext == ".002")
        ok = reader.readObjects(readEntry(zip, name));
      else if (ext == ".003")
        ok = reader.readScripts(readEntry(zip, name));
      else if (ext == ".004")
        ok = reader.readLanguages(readEntry(zip, name));
      else if (ext == ".005")
        ok = reader.readPassword(readEntry(zip, name));
      else if (ext == ".010")
        ok = reader.readAdvancedSettings(readEntry(zip, name));
      if (!ok)
        throw FormatError("not a project file entry: " + name);
    }
  }
  catch (const std::exception& e){
    if (error)
      *error = filename + ": " + e.what();
    return false;
  }
  project.versionMajor = reader.verMajor_;
  project.versionMinor = reader.verMinor_;
  return true;
}

class Writer{
public:
  explicit Writer(Project& project) : data_(project) {}

  std::string writeSettings(bool convertPngs){
    const ProjectSettings& s = data_.settings;
    TextWriter swr;
    swr.line(kWriteHeader);
    swr.line();
    swr.line(s.directory);
    swr << "Resolution X : ";
    swr.line(s.resolution.x);
    for (const FontInfo& fi : s.fonts){
      swr << fi.name << ';';
      swr << csBool(fi.bold) << ';';
      swr << csBool(fi.italic) << ';';
      swr << fi.size << ';';
      swr << fi.outline << ';';
      swr << fi.charset << ';';
      swr << fi.shadow << ';';
      swr << fi.fill << ';';
      swr << fi.fading << ';';
      swr.line(fi.spacing);
    }
    swr.line("GameFont : ");
    for (int i = 2; i <= 6; ++i)
      swr.line("GameFont" + std::to_string(i) + " : ");
    swr.line("Gameicon : " + s.gameIcon);
    swr << s.loadingImage << ";";
    swr.line(s.settingsPicture);
    swr.line(csBool(s.tsUseBgImage));
    swr.line(s.tsBackground);
    swr.line("Startskript : " + s.startScript);
    swr.line(s.mainScript);
    swr.line(s.anywhereRoom);
    swr.line("Screenchange : " + std::to_string((int)s.screenChange));
    swr << "Checks : ";
    swr << (s.textOnOff ? 1 : 0);
    swr << (s.drawDraggedItemIcons ? 1 : 0);
    swr << (s.actionText ? 1 : 0);
    swr << (s.showTaskbar ? 1 : 0);
    swr << (s.notAntialiased ? 1 : 0);
    swr << (s.taskbarFromTop ? 1 : 0);
    swr << (s.groupItems ? 1 : 0);
    swr << (s.protectGameFile ? 1 : 0);
    swr << (s.is16to9 ? 1 : 0);
    swr.line("0");
    swr.line(s.actionTextHeight);
    swr.line(csBool(s.customMenu));
    swr.line(s.customMenuRoom);
    swr.line("Taskheight : " + std::to_string(s.taskHeight));
    swr.line("Taskroom : " + s.taskRoom);
    swr.line("Taskpopup : " + std::to_string(s.taskPopup));
    swr << "Setup : ";
    swr << (s.silentDelete ? 1 : 0);
    swr << (s.infoLine ? 1 : 0);
    swr << (s.taskHideCompletely ? ";hide;" : ";show;");
    swr.line(s.rightClick);
    swr.line(csBool(s.useMouseWheel));
    swr.line(s.menuFading);
    swr.line(s.textSceneFading);
    swr.line(s.anywhereTransparency);
    swr << "Targacolor : ";
    swr.line(s.targaColor);
    swr << "Bordercolor : ";
    swr.line(s.borderColor);
    swr << "Backgroundcolor : ";
    swr.line(s.backgroundColor);
    swr << "Textcolor : ";
    swr.line(s.textColor);
    swr << "Offtextcolor : ";
    swr << s.offspeechColor << ';';
    swr.line(s.infotextColor);
    swr.line(s.tsStyle + 1);
    swr.line(s.tsBorderStyle + 1);
    swr.line(s.tsBorderColor);
    swr.line(s.tsAreaColor);
    swr.line(s.tsSelectionColor);
    swr.line(s.tsTextColor);
    swr.line(csBool(s.tsUseSymbols));
    swr.line(csBool(s.muteMusicWhenSpeech));
    swr.line(csBool(s.coinActivated));
    swr.line(csBool(s.coinAutoPopup));
    swr.line(s.coinRoom);
    swr.line(s.coinFading);
    swr.line(s.coinCenter.y);
    swr.line(s.coinCenter.x);
    for (int i = 1; i <= 25; ++i){
      swr << "dsp;" << i;
      if (i-1 < (int)s.dspEffects.size()){
        const DSPEffect& effect = s.dspEffects[i-1];
        swr << ";" << effect.name << ";";
        swr.line(effect.type);
        for (int line = 0; line < 6; ++line){
          for (int num = 0; num < 6; ++num){
            swr << effect.param[line][num];
            if (num < 5)
              swr << ";";
            else
              swr.line();
          }
        }
      }
      else{
        swr.line(";;0");
        for (int line = 0; line < 6; ++line)
          swr.line("0;0;0;0;0;0");
      }
    }
    swr << "Linktext : ";
    swr.line(s.linkText);
    swr << "Givelink : ";
    swr.line(s.giveLink);
    swr << "Walktext : ";
    swr.line(s.walkText);
    swr.line("Booleans :");
    for (const auto& boolvar : s.booleans){
      swr << boolvar.first;
      swr.line(csBool(boolvar.second));
    }
    swr.line("Commands :");
    for (const auto& cmd : s.commands){
      swr.line(cmd.first);
      swr.line(cmd.second);
    }
    swr.line("Mediapool :");
    writeTree(swr, data_.mediaPool, 0);
    swr.line("Gamepool :");
    writeTree(swr, data_.gamePool, 0);
    swr.line("Images :");
    for (const auto& image : data_.images){
      swr.line(image.key);
      if (convertPngs && extension(image.value) == ".png")
        swr.line(image.value.substr(0, image.value.size()-1) + 'j');
      else
        swr.line(image.value);
    }
    swr.line("Sounds :");
    for (const auto& sound : data_.sounds){
      swr.line(sound.key);
      swr.line(sound.value);
    }
    swr.line("Music :");
    for (const auto& music : data_.music){
      swr.line(music.key);
      swr.line(music.value);
    }
    swr.line("Movie :");
    for (const auto& video : data_.videos){
      swr.line(video.key);
      swr.line(video.value);
    }
    return swr.text();
  }

  void writeTree(TextWriter& swr, const std::vector<PoolNode>& nodes, int level){
    for (const PoolNode& node : nodes){
      for (int i = 0; i < level; ++i)
        swr << '"';
      swr << twoDigits(node.id);
      swr.line(node.label);
      writeTree(swr, node.children, level+1);
    }
  }

  //! the frame's parts in two lines, then the offsets and the script
  void writeExtendedFrame(TextWriter& swr, const ExtendedFrame* frame){
    if (!frame || frame->names.empty()){
      swr.line();
      swr.line();
    }
    else if (frame->names.size() == 1){
      swr.line(frame->names[0]);
      swr.line();
    }
    else{
      swr.line(frame->names[0]);
      swr.line(frame->names[1]);
    }
    for (int part = 0; part < kPartsMax; ++part){
      Vec2i offset;
      if (frame && part < (int)frame->offsets.size())
        offset = frame->offsets[part];
      swr << offset.x << ';' << offset.y << ';';
    }
    swr.line(frame ? replaceAll(frame->script, ';', kSemicolonStandIn) : std::string());
  }

  static const ExtendedFrame* frameAt(const std::vector<ExtendedFrame>& frames, int frame){
    return frame < (int)frames.size() ? &frames[frame] : nullptr;
  }

  std::string writeObjects(){
    TextWriter swr;
    swr.line(kWriteHeader);
    swr.line();
    for (int i = 0; i < kStatesMax; ++i){
      CursorState cs = i < (int)data_.cursor.size() ? data_.cursor[i] : CursorState();
      for (int frames = 0; frames < kFramesMax; ++frames){
        if (frames < (int)cs.frames.size())
          swr.line(cs.frames[frames]);
        else
          swr.line();
      }
      swr.line(cs.command);
      swr.line(cs.fpsDivider);
      swr.line(cs.highlight.x);
      swr.line(cs.highlight.y);
      swr.line(cs.itemOffset.x);
      swr.line(cs.itemOffset.y);
    }
    for (const auto& entry : data_.items){
      const Item& item = entry.value;
      swr << "//Item ";
      swr.line(item.name);
      for (int i = 0; i < kStatesMax; ++i){
        ItemState state = i < (int)item.states.size() ? item.states[i] : ItemState();
        for (int frames = 0; frames < kFramesMax; ++frames){
          if (frames < (int)state.frames.size())
            swr << state.frames[frames];
          swr << ';';
          std::string scr = frames < (int)state.scripts.size() ? state.scripts[frames] : "";
          swr.line(replaceAll(scr, ';', kSemicolonStandIn));
        }
        swr.line(state.fpsDivider);
      }
    }
    for (const auto& entry : data_.objects){
      const Object& obj = entry.value;
      swr << "//Object ";
      swr.line(obj.name);
      swr.line(obj.size.x);
      swr.line(obj.size.y);
      swr.line(csBool(obj.lighten));
      for (int i = 0; i < kStatesMax; ++i){
        ObjectState state = i < (int)obj.states.size() ? obj.states[i] : ObjectState();
        for (int frames = 0; frames < kFrames2Max; ++frames)
          writeExtendedFrame(swr, frameAt(state.frames, frames));
        swr.line(state.fpsDivider);
      }
    }
    for (const auto& entry : data_.characters){
      const Character& chr = entry.value;
      swr << "//Character ";
      swr.line(chr.name);
      swr.line(chr.textColor);
      swr.line(chr.walkSpeed);
      swr.line(csBool(chr.noZoom));
      swr.line(csBool(chr.realLeftAnimations));
      swr.line(csBool(chr.memoryResistant));
      swr.line(csBool(chr.ghost));
      swr.line(chr.walkSound);
      for (int i = 0; i < 20; ++i){
        if (i < (int)chr.extraStateNames.size())
          swr << chr.extraStateNames[i];
        swr << ';';
      }
      swr.line();
      swr.line(chr.font);
      swr.line(chr.zoom);
      for (int i = 0; i < kCharStatesMax; ++i){
        CharacterState state = i < (int)chr.states.size() ? chr.states[i] : CharacterState();
        swr.line(state.size.x);
        swr.line(state.size.y);
        swr.line(state.basepoint.x);
        swr.line(state.basepoint.y);
        for (int frames = 0; frames < kFrames2Max; ++frames)
          writeExtendedFrame(swr, frameAt(state.frames, frames));
        swr.line(state.fpsDivider);
      }
    }
    for (const auto& entry : data_.characterInstances){
      for (const CharacterInstance& chr : entry.value){
        swr << "//Rcharacter ";
        swr.line(chr.name);
        const Character* character = data_.characters.find(chr.character);
        swr.line(character ? character->name : chr.character);
        swr.line(chr.room);
        swr.line(chr.position.x);
        swr.line(chr.position.y);
        swr.line(chr.lookDir);
        swr.line(chr.unmovable ? 0 : -1);
        swr.line(csBool(chr.locked));
      }
    }
    for (const auto& entry : data_.rooms){
      const Room& room = entry.value;
      swr << "//Room ";
      swr.line(room.name);
      swr.line(room.size.x);
      swr.line(room.size.y);
      swr.line(room.scrollOffset.x);
      swr.line(room.scrollOffset.y);
      swr.line(room.depthmap.x);
      swr.line(room.depthmap.y);
      swr.line(room.zoom);
      swr.line(room.background);
      swr.line(room.parallaxBackground);
      swr.line(csBool(room.doubleWalkmap));
      swr << (int)room.lighting[0] << ";";
      swr << (int)room.lighting[1] << ";";
      swr.line((int)room.lighting[2]);
      for (int i = 0; i < kFxShapesMax; ++i){
        FxShape shape = i < (int)room.fxShapes.size() ? room.fxShapes[i] : FxShape::makeDefault(i);
        swr << csBool(shape.active) << ';';
        swr.line(csBool(shape.dependingOnRoomPosition));
        swr.line(shape.effect);
        swr.line(shape.room);
        swr.line(shape.depth);
        swr.line(shape.mirrorOffset.x);
        swr.line(shape.mirrorOffset.y);
        swr.line(shape.strength);
        for (int pos = 0; pos < 4; ++pos)
          swr << shape.positions[pos].x << ';' << shape.positions[pos].y << ';';
        swr.line();
      }
      swr.line(csBool(room.hasInventory));
      swr << room.invPos.x << ';' << room.invPos.y << ';';
      swr << room.invSize.x << ';' << room.invSize.y << ';';
      swr << formatFloat(room.invScale.x) << ';';
      swr << formatFloat(room.invScale.y) << ';';
      swr.line(room.invSpacing);
      for (const auto& column : room.walkmap){
        for (const WalkMapEntry& field : column){
          swr << (field.isFree ? 0 : 1);
          swr << (field.hasScript ? 1 : 0);
        }
      }
      swr.line();
      for (const ObjectInstance& obj : room.objects){
        swr << ";;Roomobject ";
        swr.line(obj.name);
        const Object* object = data_.objects.find(obj.object);
        swr.line(object ? object->name : obj.object);
        swr.line(obj.position.x);
        swr.line(obj.position.y);
        swr.line(obj.state);
        swr.line(obj.layer);
        swr.line(obj.depth/2);
        swr.line(obj.depth);
        swr.line(csBool(obj.locked));
      }
    }
    return swr.text();
  }

  void writeScript(TextWriter& swr, const char* header, const Script& scr){
    swr << header;
    swr.line(scr.name);
    swr.line(scr.text ? *scr.text : std::string());
  }

  std::string writeScripts(){
    TextWriter swr;
    swr.line(kWriteHeader);
    swr.line();
    for (const auto& entry : data_.scripts[Script::Cutscene])
      writeScript(swr, "//Cutscene ", entry.value);
    for (const auto& entry : data_.scripts[Script::Item])
      writeScript(swr, "//Item ", entry.value);
    for (const auto& entry : data_.scripts[Script::Character])
      writeScript(swr, "//Character ", entry.value);
    for (const auto& entry : data_.rooms){
      const Room& room = entry.value;
      const Script* scr = data_.scripts[Script::Room].find(room.name);
      if (scr)
        writeScript(swr, "//Room ", *scr);
      else{
        swr << "//Room ";
        swr.line(room.name);
      }
      for (const ObjectInstance& obj : room.objects){
        const Script* oscr = data_.scripts[Script::Object].find(Script::objectScriptName(obj.name, room.name));
        if (oscr){
          swr << "//Object ";
          swr.line(oscr->name);
          if (oscr->text)
            swr.line(*oscr->text);
        }
      }
      for (const CharacterInstance& chr : data_.roomCharacters(room.name)){
        if (!data_.scripts[Script::Character].find(chr.name)){
          swr << "//Character ";
          swr.line(chr.name);
        }
      }
      for (size_t x = 0; x < room.walkmap.size(); ++x){
        for (size_t y = 0; y < room.walkmap[x].size(); ++y){
          if (!room.walkmap[x][y].hasScript)
            continue;
          const Script* wms = data_.scripts[Script::Walkmap].find(Script::walkmapScriptName((int)x, (int)y, room.name));
          if (wms)
            writeScript(swr, "//Walkmap ", *wms);
        }
      }
    }
    return swr.text();
  }

  std::string writeLanguages(){
    static const char* const sectionNames[Language::NumSections] = {
      "speech", "voicespeech", "offspeech", "voiceoffspeech", "showinfo",
      "textout", "setstring", "row", "kommandos"
    };
    TextWriter swr;
    for (const auto& entry : data_.languages){
      const Language& language = entry.value;
      std::string lang = "*/*" + language.name + ';';
      for (int section = 0; section < Language::NumSections; ++section){
        swr.line(lang + sectionNames[section]);
        for (const std::string& str : language.sections[section])
          swr.line(str);
      }
    }
    return swr.text();
  }

  std::string writePassword(){
    if (data_.zipPassword.size() < 20){
      std::random_device seed;
      std::mt19937 rand(seed());
      std::uniform_int_distribution<int> chars(33, 126);
      data_.zipPassword.clear();
      for (int i = 0; i < 20; ++i)
        data_.zipPassword += (char)chars(rand);
    }
    std::string pwdwrite;
    for (int i = 0; i < 20; ++i){
      int val = (unsigned char)data_.zipPassword[i];
      if (i >= 15)
        val *= 3;
      else if (i >= 10)
        val *= 6;
      else if (i >= 5)
        val *= 4;
      else
        val *= 5;
      pwdwrite += std::to_string(val);
    }
    TextWriter swr;
    swr.line(pwdwrite);
    return swr.text();
  }

  std::string writeAdvancedSettings(){
    TextWriter swr;
    swr.line(kWriteHeader);
    swr.line();
    swr << csBool(data_.settings.pngToJpeg);
    swr.line("");
    swr.line(data_.settings.scriptingLanguage == ScriptLang::Lua ? "LUA" : "PCDK");
    return swr.text();
  }

private:
  Project& data_;
};

}

bool importObjects(const std::string& text, Project& project, std::vector<std::string>* createdObjects, std::string* error){
  Reader reader(project);
  reader.createdObjects_ = createdObjects;
  try{
    LineReader rdr(text);
    return reader.readObjectsLoop(rdr);
  }
  catch (const std::exception& e){
    if (error)
      *error = e.what();
    return false;
  }
}

bool importScripts(const std::string& text, Project& project, std::string* error){
  Reader reader(project);
  try{
    LineReader rdr(text);
    return reader.readScriptsLoop(rdr);
  }
  catch (const std::exception& e){
    if (error)
      *error = e.what();
    return false;
  }
}

bool readProject(const std::string& filename, Project& project, std::string* error){
  CGE::ZipReader zip(filename);
  return readZip(zip, filename, project, error);
}

bool readProject(const void* data, size_t size, const std::string& filename, Project& project, std::string* error){
  CGE::ZipReader zip(const_cast<void*>(data), (uint32)size);
  return readZip(zip, filename, project, error);
}

std::vector<std::pair<std::string, std::string>> writeProjectEntries(Project& project, bool dat){
  Writer writer(project);
  std::vector<std::pair<std::string, std::string>> entries;
  entries.push_back(std::make_pair(".001", writer.writeSettings(dat && project.settings.pngToJpeg)));
  entries.push_back(std::make_pair(".002", writer.writeObjects()));
  entries.push_back(std::make_pair(".003", writer.writeScripts()));
  entries.push_back(std::make_pair(".004", writer.writeLanguages()));
  if (project.settings.protectGameFile)
    entries.push_back(std::make_pair(".005", writer.writePassword()));
  entries.push_back(std::make_pair(".010", writer.writeAdvancedSettings()));
  return entries;
}

bool writeProject(const std::string& filename, Project& project, std::string* error){
  std::vector<std::pair<std::string, std::string>> entries = writeProjectEntries(project, extension(filename) == ".dat");
  CGE::ZipWriter zip(filename);
  if (!zip.isWorking()){
    if (error)
      *error = "cannot write " + filename;
    return false;
  }
  std::string name = baseName(filename);
  for (auto& entry : entries){
    CGE::MemReader rdr(entry.second.empty() ? nullptr : &entry.second[0], (unsigned)entry.second.size());
    if (!zip.addEntry(name + entry.first, rdr)){
      if (error)
        *error = "cannot write " + filename;
      return false;
    }
  }
  return true;
}

}
