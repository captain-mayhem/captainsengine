#include "roomedit.h"

#include <algorithm>
#include <cmath>

namespace sd{

namespace{

const CharacterState* stateOf(const Project& project, const CharacterInstance& chr){
  const Character* c = project.characters.find(chr.character);
  if (c == nullptr || c->states.empty())
    return nullptr;
  size_t state = (size_t)characterState(chr);
  return &c->states[std::min(state, c->states.size()-1)];
}

Vec2i basepoint(const Project& project, const CharacterInstance& chr){
  const CharacterState* state = stateOf(project, chr);
  return state ? state->basepoint : Vec2i{};
}

std::vector<CharacterInstance>* roomChars(Project& project, const std::string& room){
  return project.characterInstances.find(room);
}

bool objectNameTaken(const Room& room, const std::string& name){
  for (const ObjectInstance& obj : room.objects){
    if (toLower(obj.name) == toLower(name))
      return true;
  }
  return false;
}

bool characterNameTaken(const Project& project, const std::string& name){
  for (const auto& room : project.characterInstances){
    for (const CharacterInstance& chr : room.value){
      if (toLower(chr.name) == toLower(name))
        return true;
    }
  }
  return false;
}

template<typename T>
size_t moveTo(std::vector<T>& list, size_t index, bool front){
  if (index >= list.size())
    return index;
  T moved = list[index];
  list.erase(list.begin() + index);
  if (front){
    list.push_back(moved);
    return list.size()-1;
  }
  list.insert(list.begin(), moved);
  return 0;
}

int clampInt(int value, int low, int high){
  return value < low ? low : (value > high ? high : value);
}

}

int windowWidth(const ProjectSettings& settings){
  int x = settings.resolution.x;
  if (settings.is16to9){
    switch (x){
      case 320: return 420;
      case 640: return 840;
      case 800: return 1050;
      case 1024: return 1344;
      case 1440: return 1890;
    }
  }
  return x;
}

float roomScale(const Project& project, const Room& room, Vec2i pos){
  const int grid = project.walkGridSize();
  float scaleStart = (float)(room.depthmap.y*grid);
  float scaleStop = (float)(room.depthmap.x*grid);
  float minVal = 1.0f - (scaleStart-scaleStop)/project.settings.resolution.y*room.zoom*0.3f;
  float factor = (pos.y - scaleStart) / (scaleStop - scaleStart);
  factor = factor < 0 ? 0 : factor;
  factor = factor > 1.0f ? 1.0f : factor;
  return (1 - factor) + factor*minVal;
}

int objectDepth(const ObjectInstance& obj){
  if (obj.layer == 0)
    return 0;
  if (obj.layer == 1)
    return obj.depth;
  return 999;
}

int characterDepth(const Project& project, const CharacterInstance& chr){
  int grid = project.walkGridSize();
  return grid == 0 ? 0 : characterFoot(project, chr).y*2/grid;
}

int characterState(const CharacterInstance& chr){
  int state = chr.lookDir == 4 ? 3 : chr.lookDir;
  return state < 1 ? 0 : state-1;
}

Vec2i characterFoot(const Project& project, const CharacterInstance& chr){
  // the C# editor adds the basepoint of the state after the one shown here,
  // a slip; the game and the drawing take the state's own
  Vec2i spot = basepoint(project, chr);
  return Vec2i{chr.position.x + spot.x, chr.position.y + spot.y};
}

void setCharacterFoot(const Project& project, CharacterInstance& chr, Vec2i foot){
  Vec2i spot = basepoint(project, chr);
  chr.position = Vec2i{foot.x - spot.x, foot.y - spot.y};
}

void setLookDir(const Project& project, CharacterInstance& chr, int lookDir){
  Vec2i foot = characterFoot(project, chr);
  chr.lookDir = lookDir;
  setCharacterFoot(project, chr, foot);
}

std::vector<RoomEntry> drawOrder(const Project& project, const Room& room){
  std::vector<RoomEntry> order;
  auto insert = [&order](const RoomEntry& entry){
    // after the entries of the same depth, as the C# blit queue
    auto pos = std::upper_bound(order.begin(), order.end(), entry,
      [](const RoomEntry& a, const RoomEntry& b){return a.depth < b.depth;});
    order.insert(pos, entry);
  };
  for (size_t i = 0; i < room.objects.size(); ++i)
    insert(RoomEntry{false, i, objectDepth(room.objects[i])});
  const std::vector<CharacterInstance>& chars = project.roomCharacters(room.name);
  for (size_t i = 0; i < chars.size(); ++i)
    insert(RoomEntry{true, i, characterDepth(project, chars[i])});
  return order;
}

void entryBox(const Project& project, const Room& room, const RoomEntry& entry, Vec2i& pos, Vec2i& size){
  if (entry.character){
    const CharacterInstance& chr = project.roomCharacters(room.name)[entry.index];
    const CharacterState* state = stateOf(project, chr);
    pos = chr.position;
    size = state ? state->size : Vec2i{};
  }
  else{
    const ObjectInstance& obj = room.objects[entry.index];
    const Object* o = project.objects.find(obj.object);
    pos = obj.position;
    size = o ? o->size : Vec2i{};
  }
}

std::optional<RoomEntry> entryAt(const Project& project, const Room& room, Vec2i pos, bool second){
  std::optional<RoomEntry> ret, ret2;
  int retdepth = -1;
  int secretdepth = -1;
  auto hit = [&](const RoomEntry& entry){
    Vec2i at, size;
    entryBox(project, room, entry, at, size);
    int x = pos.x - at.x;
    int y = pos.y - at.y;
    return x >= 0 && y >= 0 && x <= size.x && y <= size.y;
  };
  // objects in front win over earlier ones of their depth, characters don't
  for (size_t i = 0; i < room.objects.size(); ++i){
    RoomEntry entry{false, i, objectDepth(room.objects[i])};
    if (hit(entry) && entry.depth >= retdepth){
      secretdepth = retdepth;
      retdepth = entry.depth;
      ret2 = ret;
      ret = entry;
    }
  }
  const std::vector<CharacterInstance>& chars = project.roomCharacters(room.name);
  for (size_t i = 0; i < chars.size(); ++i){
    RoomEntry entry{true, i, characterDepth(project, chars[i])};
    if (!hit(entry))
      continue;
    if (entry.depth > retdepth){
      secretdepth = retdepth;
      retdepth = entry.depth;
      ret2 = ret;
      ret = entry;
    }
    else if (entry.depth > secretdepth){
      secretdepth = entry.depth;
      ret2 = entry;
    }
  }
  return second ? ret2 : ret;
}

ObjectInstance& placeObject(Project& project, Room& room, const std::string& object, Vec2i pos){
  const Object* obj = project.objects.find(object);
  ObjectInstance inst;
  inst.object = obj ? obj->name : object;
  int count = 0;
  for (const ObjectInstance& o : room.objects){
    if (toLower(o.object) == toLower(object))
      ++count;
  }
  // numbered by the instances there, as the C# editor; past names already taken
  inst.name = inst.object + (count > 0 ? std::to_string(count) : "");
  while (objectNameTaken(room, inst.name))
    inst.name = inst.object + std::to_string(++count);
  int grid = project.walkGridSize();
  inst.depth = grid == 0 ? 1 : pos.y*2/grid + 1;
  inst.layer = 2;
  inst.state = 1;
  inst.position = pos;
  room.objects.push_back(inst);
  return room.objects.back();
}

CharacterInstance& placeCharacter(Project& project, const std::string& room, const std::string& character, Vec2i pos){
  const Character* chr = project.characters.find(character);
  CharacterInstance inst;
  inst.character = chr ? chr->name : character;
  int count = 0;
  for (const auto& r : project.characterInstances){
    for (const CharacterInstance& c : r.value){
      if (toLower(c.character) == toLower(character))
        ++count;
    }
  }
  inst.name = inst.character + (count > 0 ? std::to_string(count) : "");
  while (characterNameTaken(project, inst.name))
    inst.name = inst.character + std::to_string(++count);
  inst.room = room;
  inst.lookDir = 1;
  inst.position = pos;
  if (roomChars(project, room) == nullptr)
    project.characterInstances.add(room, std::vector<CharacterInstance>());
  std::vector<CharacterInstance>& chars = *roomChars(project, room);
  chars.push_back(inst);
  return chars.back();
}

void removeObjectInstance(Project& project, Room& room, size_t index){
  if (index >= room.objects.size())
    return;
  project.scripts[Script::Object].remove(Script::objectScriptName(room.objects[index].name, room.name));
  room.objects.erase(room.objects.begin() + index);
}

void removeCharacterInstance(Project& project, const std::string& room, size_t index){
  std::vector<CharacterInstance>* chars = roomChars(project, room);
  if (chars == nullptr || index >= chars->size())
    return;
  project.scripts[Script::Character].remove((*chars)[index].name);
  chars->erase(chars->begin() + index);
}

size_t moveToBack(Project& project, Room& room, const RoomEntry& entry){
  if (!entry.character)
    return moveTo(room.objects, entry.index, false);
  std::vector<CharacterInstance>* chars = roomChars(project, room.name);
  return chars ? moveTo(*chars, entry.index, false) : entry.index;
}

size_t moveToFront(Project& project, Room& room, const RoomEntry& entry){
  if (!entry.character)
    return moveTo(room.objects, entry.index, true);
  std::vector<CharacterInstance>* chars = roomChars(project, room.name);
  return chars ? moveTo(*chars, entry.index, true) : entry.index;
}

int objectDepthAt(const Project& project, const Room& room, int y){
  const int grid = project.walkGridSize();
  if (grid < 2)
    return 1;
  int depth = (int)((y + 5 + grid/4.0f) / (grid/2.0f));
  int most = room.size.y/(grid/2) - 2;
  if (depth > most)
    depth = most;
  if (depth < 1)
    depth = 1;
  return depth;
}

int walkmapScale(const Room& room){
  return room.doubleWalkmap ? 2 : 1;
}

Vec2i walkmapField(const Project& project, const Room& room, Vec2i pos){
  const int grid = project.walkGridSize();
  if (grid == 0)
    return Vec2i{};
  const int scale = walkmapScale(room);
  // as the C#'s integer division, which rounds towards zero
  return Vec2i{scale*pos.x/grid, scale*pos.y/grid};
}

bool inWalkmap(const Room& room, Vec2i field){
  return field.x >= 0 && field.y >= 0 && (size_t)field.x < room.walkmap.size() && (size_t)field.y < room.walkmap[field.x].size();
}

void paintWalkmap(Room& room, Vec2i field, WalkmapPen pen, bool free){
  auto set = [&room, free](int x, int y){
    if (inWalkmap(room, Vec2i{x, y}))
      room.walkmap[x][y].isFree = free;
  };
  const int x = field.x;
  const int y = field.y;
  set(x, y);
  if (pen == WalkmapPen::Size2 || pen == WalkmapPen::Size3){
    set(x+1, y);
    set(x-1, y);
    set(x, y+1);
    set(x, y-1);
  }
  if (pen == WalkmapPen::Size3){
    set(x+1, y-1);
    set(x-1, y+1);
    set(x-1, y-1);
    set(x+1, y+1);
    set(x+2, y);
    set(x-2, y);
    set(x, y+2);
    set(x, y-2);
  }
}

Script& walkmapScript(Project& project, Room& room, Vec2i field){
  std::string name = Script::walkmapScriptName(field.x, field.y, room.name);
  if (inWalkmap(room, field))
    room.walkmap[field.x][field.y].hasScript = true;
  NamedList<Script>& scripts = project.scripts[Script::Walkmap];
  Script* scr = scripts.find(name);
  if (scr == nullptr){
    Script created;
    created.type = Script::Walkmap;
    created.name = name;
    created.text = std::string();
    scripts.add(name, created);
    scr = scripts.find(name);
  }
  return *scr;
}

void pasteWalkmapScript(Project& project, Room& room, Vec2i field, const std::optional<std::string>& text){
  if (!inWalkmap(room, field))
    return;
  walkmapScript(project, room, field).text = text;
}

void eraseWalkmapScript(Project& project, Room& room, Vec2i field){
  if (!inWalkmap(room, field) || !room.walkmap[field.x][field.y].hasScript)
    return;
  project.scripts[Script::Walkmap].remove(Script::walkmapScriptName(field.x, field.y, room.name));
  room.walkmap[field.x][field.y].hasScript = false;
}

int depthmapLineAt(const Project& project, const Room& room, int y){
  const int grid = project.walkGridSize();
  return grid == 0 ? 0 : (y + grid/2) / grid;
}

void setDepthmapLine(const Project& project, Room& room, int line, int depth){
  const int grid = project.walkGridSize();
  if (line == 0){
    if (depth >= room.depthmap.y)
      depth = room.depthmap.y - 1;
    if (depth < 1)
      depth = 1;
    room.depthmap.x = depth;
  }
  else{
    if (grid != 0 && depth >= room.size.y/grid)
      depth = room.size.y/grid - 1;
    if (depth <= room.depthmap.x)
      depth = room.depthmap.x + 1;
    room.depthmap.y = depth;
  }
}

Vec2i inventoryPixels(const Room& room){
  return Vec2i{room.invSize.x*((int)(50*room.invScale.x) + room.invSpacing) + 30,
    room.invSize.y*((int)(50*room.invScale.y) + room.invSpacing) + 5};
}

void dragInventoryCorner(const Project& project, Room& room, Vec2i pos){
  const Vec2i res = project.settings.resolution;
  const int maxX = (res.x - 30) / 60;
  const int maxY = (res.y - 5) / 60;
  room.invSize.x = (int)std::round((pos.x - room.invPos.x - 30) / (room.invScale.x*50 + room.invSpacing));
  room.invSize.x = clampInt(room.invSize.x, 1, std::max(maxX, 1));
  room.invSize.y = (int)std::round((pos.y - room.invPos.y - 5) / (room.invScale.y*50 + room.invSpacing));
  room.invSize.y = clampInt(room.invSize.y, 1, std::max(maxY, 1));
}

void scaleInventory(Room& room, int axis, float pixels){
  // the size the fields have unscaled
  float unscaled = axis == 0 ? room.invSize.x*(50.0f + room.invSpacing) + 30 : room.invSize.y*(50.0f + room.invSpacing) + 5;
  float& scale = axis == 0 ? room.invScale.x : room.invScale.y;
  scale = pixels / unscaled;
  if (scale > 1.5f)
    scale = 1.5f;
  if (scale < 0.3f)
    scale = 0.3f;
}

int fxDepthHandleY(const Project& project, const FxShape& shape){
  // where the C# editor takes the handle; it draws it half as far down
  const int grid = project.walkGridSize();
  return (int)((shape.depth + 0.5f)*grid - grid/2);
}

int fxDepthAt(const Project& project, const Room& room, int y){
  const int grid = project.walkGridSize();
  if (grid == 0)
    return 1;
  return clampInt((y + 5 + grid/2) / grid, 1, std::max(room.size.y/grid, 1));
}

Vec2i fxCenter(const FxShape& shape){
  Vec2i center;
  for (const Vec2i& pos : shape.positions){
    center.x += pos.x;
    center.y += pos.y;
  }
  return Vec2i{center.x/4, center.y/4};
}

void moveFxShape(const Room& room, FxShape& shape, Vec2i diff){
  for (const Vec2i& pos : shape.positions){
    diff.x = clampInt(diff.x, -pos.x, room.size.x - pos.x);
    diff.y = clampInt(diff.y, -pos.y, room.size.y - pos.y);
  }
  for (Vec2i& pos : shape.positions){
    pos.x += diff.x;
    pos.y += diff.y;
  }
}

void setFxCorner(const Room& room, FxShape& shape, int corner, Vec2i pos){
  if (corner < 0 || corner >= 4)
    return;
  shape.positions[corner] = Vec2i{clampInt(pos.x, 0, room.size.x), clampInt(pos.y, 0, room.size.y)};
}

}
