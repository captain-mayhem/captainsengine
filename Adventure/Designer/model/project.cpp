#include "project.h"

#include <cstdio>

namespace sd{

std::string toLower(const std::string& text){
  std::string ret = text;
  for (char& c : ret){
    unsigned char ch = (unsigned char)c;
    if (ch >= 'A' && ch <= 'Z')
      ch += 'a' - 'A';
    //the Latin-1 capitals, but the multiplication sign
    else if (ch >= 0xC0 && ch <= 0xDE && ch != 0xD7)
      ch += 0x20;
    //S, OE and Z with caron and Y with diaeresis, which 1252 has elsewhere
    else if (ch == 0x8A || ch == 0x8C || ch == 0x8E)
      ch += 0x10;
    else if (ch == 0x9F)
      ch = 0xFF;
    c = (char)ch;
  }
  return ret;
}

namespace{

//! the code points of Windows-1252's 0x80 to 0x9F (the unused ones as in Latin-1)
const uint16_t kCp1252High[32] = {
  0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
  0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
};

void appendUtf8(std::string& out, uint32_t cp){
  if (cp < 0x80)
    out += (char)cp;
  else if (cp < 0x800){
    out += (char)(0xC0 | (cp >> 6));
    out += (char)(0x80 | (cp & 0x3F));
  }
  else{
    out += (char)(0xE0 | (cp >> 12));
    out += (char)(0x80 | ((cp >> 6) & 0x3F));
    out += (char)(0x80 | (cp & 0x3F));
  }
}

}

std::string toUtf8(const std::string& text){
  std::string ret;
  ret.reserve(text.size());
  for (char c : text){
    unsigned char ch = (unsigned char)c;
    if (ch >= 0x80 && ch < 0xA0)
      appendUtf8(ret, kCp1252High[ch-0x80]);
    else
      appendUtf8(ret, ch);
  }
  return ret;
}

std::string fromUtf8(const std::string& text){
  std::string ret;
  ret.reserve(text.size());
  for (size_t i = 0; i < text.size();){
    unsigned char ch = (unsigned char)text[i];
    uint32_t cp;
    int len;
    if (ch < 0x80){
      cp = ch;
      len = 1;
    }
    else if ((ch & 0xE0) == 0xC0){
      cp = ch & 0x1F;
      len = 2;
    }
    else if ((ch & 0xF0) == 0xE0){
      cp = ch & 0x0F;
      len = 3;
    }
    else{
      cp = ch & 0x07;
      len = 4;
    }
    for (int k = 1; k < len && i+k < text.size(); ++k)
      cp = (cp << 6) | ((unsigned char)text[i+k] & 0x3F);
    i += len;
    char out = '?';
    if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF))
      out = (char)cp;
    else{
      for (int k = 0; k < 32; ++k){
        if (kCp1252High[k] == cp)
          out = (char)(0x80+k);
      }
    }
    ret += out;
  }
  return ret;
}

FxShape FxShape::makeDefault(int num){
  FxShape shape;
  shape.active = false;
  shape.dependingOnRoomPosition = true;
  shape.effect = FloorMirror;
  shape.depth = 15;
  shape.mirrorOffset = Vec2i{-10, -7};
  shape.strength = 125;
  shape.positions[0] = Vec2i{105*(num+1), 100};
  shape.positions[1] = Vec2i{shape.positions[0].x+100, 100};
  shape.positions[2] = Vec2i{shape.positions[1].x, 200};
  shape.positions[3] = Vec2i{shape.positions[0].x, 200};
  return shape;
}

std::string Script::objectScriptName(const std::string& object, const std::string& room){
  return object + ";" + room;
}

std::string Script::walkmapScriptName(int x, int y, const std::string& room){
  char buf[32];
  snprintf(buf, sizeof(buf), "#%03d%03d", x+1, y+1);
  return buf + room;
}

Project::Project(){
  //a new project's cursor: ten states, the last one for the second command
  for (int i = 0; i < 10; ++i){
    CursorState state;
    state.command = i == 9 ? 2 : 1;
    cursor.push_back(state);
  }
}

Vec2i Project::walkmapSize() const{
  Vec2i grid = settings.resolution;
  int size = walkGridSize();
  if (size == 0)
    return Vec2i();
  grid.x /= size;
  grid.y /= size;
  return Vec2i{grid.x*3*2, grid.y*2*2};
}

const std::vector<CharacterInstance>& Project::roomCharacters(const std::string& room) const{
  static const std::vector<CharacterInstance> none;
  const std::vector<CharacterInstance>* chars = characterInstances.find(room);
  return chars ? *chars : none;
}

}
