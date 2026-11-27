#include "resources.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <random>

#include "images.h"
#include "roomedit.h"

namespace fs = std::filesystem;

namespace sd{

namespace{

PoolNode folderNode(const std::string& label){
  PoolNode node;
  node.id = (int)ResourceID::Folder;
  node.label = label;
  return node;
}

std::vector<PoolNode>& poolOf(Project& project, bool game){
  return game ? project.gamePool : project.mediaPool;
}

//! the list that holds the node at path (the pool itself for a top folder)
std::vector<PoolNode>* parentList(std::vector<PoolNode>& pool, const PoolPath& path){
  if (path.empty())
    return nullptr;
  if (path.size() == 1)
    return &pool;
  PoolPath parent(path.begin(), path.end()-1);
  PoolNode* node = poolNode(pool, parent);
  return node ? &node->children : nullptr;
}

std::vector<PoolNode>* listHolding(std::vector<PoolNode>& nodes, ResourceID kind, const std::string& name){
  for (PoolNode& node : nodes){
    if (node.id == (int)kind && toLower(node.label) == toLower(name))
      return &nodes;
    if (std::vector<PoolNode>* found = listHolding(node.children, kind, name))
      return found;
  }
  return nullptr;
}

bool pathTo(const std::vector<PoolNode>& nodes, ResourceID kind, const std::string& name, PoolPath& path){
  for (size_t i = 0; i < nodes.size(); ++i){
    path.push_back(i);
    if (nodes[i].id == (int)kind && toLower(nodes[i].label) == toLower(name))
      return true;
    if (pathTo(nodes[i].children, kind, name, path))
      return true;
    path.pop_back();
  }
  return false;
}

//! renames a resource's node and sorts its folder
void renameInPool(Project& project, ResourceID kind, const std::string& from, const std::string& to){
  std::vector<PoolNode>& pool = poolOf(project, inGamePool(kind));
  if (std::vector<PoolNode>* list = listHolding(pool, kind, from)){
    for (PoolNode& node : *list){
      if (node.id == (int)kind && toLower(node.label) == toLower(from))
        node.label = to;
    }
    sortPool(*list);
  }
}

//! a script under a new name, if there is one
void renameScript(Project& project, Script::Type type, const std::string& from, const std::string& to){
  NamedList<Script>& scripts = project.scripts[type];
  Script* scr = scripts.find(from);
  if (scr == nullptr)
    return;
  Script copy = *scr;
  copy.name = to;
  scripts.remove(from);
  scripts.set(to, copy);
}

//! a copy of a script under a new name, if there is one
void copyScript(Project& project, Script::Type type, const std::string& from, const std::string& to){
  const Script* scr = project.scripts[type].find(from);
  if (scr == nullptr)
    return;
  Script copy = *scr;
  copy.name = to;
  project.scripts[type].set(to, copy);
}

//! the walkmap fields with scripts
template<typename F>
void forWalkmapScripts(const Room& room, F f){
  for (size_t x = 0; x < room.walkmap.size(); ++x){
    for (size_t y = 0; y < room.walkmap[x].size(); ++y){
      if (room.walkmap[x][y].hasScript)
        f((int)x, (int)y);
    }
  }
}

template<typename T>
bool takenByOther(const NamedList<T>& list, const std::string& from, const std::string& to){
  return toLower(from) != toLower(to) && list.find(to) != nullptr;
}

//! a list's entry under a new name, keeping its place for an unchanged name
template<typename T>
void renameEntry(NamedList<T>& list, const std::string& from, const std::string& to){
  T* entry = list.find(from);
  if (entry == nullptr)
    return;
  T copy = *entry;
  copy.name = to;
  //as the C# StoryDesigner: taken out and added again with the new name
  list.remove(from);
  list.add(to, copy);
}

}

void newPools(Project& project){
  project.gamePool.clear();
  for (const char* name : {"1) CHARACTERS", "2) SCRIPTS", "3) ITEMS", "4) OBJECTS", "5) ROOMS"})
    project.gamePool.push_back(folderNode(name));
  project.mediaPool.clear();
  for (const char* name : {"IMAGES", "MUSIC", "SOUNDS", "VIDEOS"})
    project.mediaPool.push_back(folderNode(name));
}

bool inGamePool(ResourceID kind){
  switch (kind){
    case ResourceID::Image:
    case ResourceID::Music:
    case ResourceID::Sound:
    case ResourceID::Video:
      return false;
    default:
      return true;
  }
}

int topFolder(ResourceID kind){
  switch (kind){
    case ResourceID::Character: return 0;
    case ResourceID::Script: return 1;
    case ResourceID::Item: return 2;
    case ResourceID::Object: return 3;
    case ResourceID::Room: return 4;
    case ResourceID::Image: return 0;
    case ResourceID::Music: return 1;
    case ResourceID::Sound: return 2;
    case ResourceID::Video: return 3;
    case ResourceID::Folder: break;
  }
  return -1;
}

ResourceID kindAt(const std::vector<PoolNode>& pool, bool game, const PoolPath& path){
  if (path.empty())
    return ResourceID::Folder;
  static const ResourceID gameKinds[] = {ResourceID::Character, ResourceID::Script, ResourceID::Item, ResourceID::Object, ResourceID::Room};
  static const ResourceID mediaKinds[] = {ResourceID::Image, ResourceID::Music, ResourceID::Sound, ResourceID::Video};
  size_t top = path[0];
  if (game && top < 5)
    return gameKinds[top];
  if (!game && top < 4)
    return mediaKinds[top];
  return ResourceID::Folder;
}

PoolNode* poolNode(std::vector<PoolNode>& pool, const PoolPath& path){
  std::vector<PoolNode>* level = &pool;
  PoolNode* node = nullptr;
  for (size_t idx : path){
    if (idx >= level->size())
      return nullptr;
    node = &(*level)[idx];
    level = &node->children;
  }
  return node;
}

PoolNode* findInPool(std::vector<PoolNode>& pool, ResourceID kind, const std::string& name){
  for (PoolNode& node : pool){
    if (node.id == (int)kind && toLower(node.label) == toLower(name))
      return &node;
    if (PoolNode* found = findInPool(node.children, kind, name))
      return found;
  }
  return nullptr;
}

PoolPath pathInPool(const std::vector<PoolNode>& pool, ResourceID kind, const std::string& name){
  PoolPath path;
  if (!pathTo(pool, kind, name, path))
    path.clear();
  return path;
}

void sortPool(std::vector<PoolNode>& nodes){
  std::stable_sort(nodes.begin(), nodes.end(), [](const PoolNode& a, const PoolNode& b){
    return toLower(a.label) < toLower(b.label);
  });
}

bool removeFromPool(std::vector<PoolNode>& pool, ResourceID kind, const std::string& name){
  for (size_t i = 0; i < pool.size(); ++i){
    if (pool[i].id == (int)kind && toLower(pool[i].label) == toLower(name)){
      pool.erase(pool.begin()+i);
      return true;
    }
    if (removeFromPool(pool[i].children, kind, name))
      return true;
  }
  return false;
}

PoolPath targetFolder(Project& project, ResourceID kind, const PoolPath& folder){
  bool game = inGamePool(kind);
  std::vector<PoolNode>& pool = poolOf(project, game);
  int top = topFolder(kind);
  if (top < 0)
    return PoolPath();
  //a project without its top folders (read from a broken file) gets them
  if (pool.size() <= (size_t)top){
    std::vector<PoolNode> saved = pool;
    Project fresh;
    newPools(fresh);
    std::vector<PoolNode>& tops = game ? fresh.gamePool : fresh.mediaPool;
    for (size_t i = saved.size(); i < tops.size(); ++i)
      pool.push_back(tops[i]);
  }
  //the folder itself, or the folder of a resource in it
  PoolPath path = folder;
  PoolNode* node = poolNode(pool, path);
  if (node && node->id != (int)ResourceID::Folder){
    path.pop_back();
    node = path.empty() ? nullptr : poolNode(pool, path);
  }
  if (node && node->id == (int)ResourceID::Folder && kindAt(pool, game, path) == kind)
    return path;
  return PoolPath{(size_t)top};
}

void addToPool(Project& project, ResourceID kind, const std::string& name, const PoolPath& folder){
  PoolPath target = targetFolder(project, kind, folder);
  PoolNode* parent = poolNode(poolOf(project, inGamePool(kind)), target);
  if (parent == nullptr)
    return;
  PoolNode node;
  node.id = (int)kind;
  node.label = name;
  parent->children.push_back(node);
  sortPool(parent->children);
}

std::string newResource(Project& project, ResourceID kind, const PoolPath& folder){
  std::string name;
  switch (kind){
    case ResourceID::Item:{
      Item item;
      name = "Item" + std::to_string(project.items.size()+1);
      if (project.items.find(name))
        name = freeName(project.items, "Item");
      item.name = name;
      item.states.resize(10);
      project.items.add(name, item);
      break;
    }
    case ResourceID::Object:{
      Object obj;
      name = "Object" + std::to_string(project.objects.size()+1);
      if (project.objects.find(name))
        name = freeName(project.objects, "Object");
      obj.name = name;
      obj.size = Vec2i{100, 100};
      obj.states.resize(10);
      project.objects.add(name, obj);
      break;
    }
    case ResourceID::Character:{
      Character chr;
      name = "Character" + std::to_string(project.characters.size()+1);
      if (project.characters.find(name))
        name = freeName(project.characters, "Character");
      chr.name = name;
      chr.walkSpeed = 5;
      chr.zoom = 100;
      //a random text colour, as the C# StoryDesigner
      static std::mt19937 rand{std::random_device{}()};
      std::uniform_int_distribution<uint32_t> part(0, 254);
      chr.textColor = part(rand) << 16 | part(rand) << 8 | part(rand);
      chr.extraStateNames.resize(36-16);
      chr.states.resize(36);
      for (CharacterState& state : chr.states)
        state.basepoint = Vec2i{60, 198};
      project.characters.add(name, chr);
      break;
    }
    case ResourceID::Room:{
      Room room;
      name = "Room" + std::to_string(project.rooms.size()+1);
      if (project.rooms.find(name))
        name = freeName(project.rooms, "Room");
      room.name = name;
      room.size = Vec2i{windowWidth(project.settings), project.settings.resolution.y};
      room.depthmap = Vec2i{5, 10};
      room.zoom = 3;
      for (int i = 0; i < 3; ++i)
        room.fxShapes.push_back(FxShape::makeDefault(i));
      Vec2i wm = project.walkmapSize();
      room.walkmap.assign(wm.x, std::vector<WalkMapEntry>(wm.y));
      room.invScale = Vec2f{1, 1};
      room.invSpacing = 10;
      project.rooms.add(name, room);
      break;
    }
    case ResourceID::Script:{
      NamedList<Script>& scripts = project.scripts[Script::Cutscene];
      name = "Script" + std::to_string(scripts.size()+1);
      if (scripts.find(name))
        name = freeName(scripts, "Script");
      Script scr;
      scr.type = Script::Cutscene;
      scr.name = name;
      scr.text = std::string();
      scripts.add(name, scr);
      break;
    }
    default:
      return std::string();
  }
  addToPool(project, kind, name, folder);
  return name;
}

std::string duplicateResource(Project& project, ResourceID kind, const std::string& name, const PoolPath& folder){
  std::string newName;
  switch (kind){
    case ResourceID::Item:{
      const Item* item = project.items.find(name);
      if (item == nullptr)
        return std::string();
      Item copy = *item;
      newName = freeName(project.items, item->name);
      copy.name = newName;
      project.items.add(newName, copy);
      copyScript(project, Script::Item, name, newName);
      break;
    }
    case ResourceID::Object:{
      const Object* obj = project.objects.find(name);
      if (obj == nullptr)
        return std::string();
      Object copy = *obj;
      newName = freeName(project.objects, obj->name);
      copy.name = newName;
      project.objects.add(newName, copy);
      break;
    }
    case ResourceID::Character:{
      const Character* chr = project.characters.find(name);
      if (chr == nullptr)
        return std::string();
      Character copy = *chr;
      newName = freeName(project.characters, chr->name);
      copy.name = newName;
      project.characters.add(newName, copy);
      break;
    }
    case ResourceID::Script:{
      const Script* scr = project.scripts[Script::Cutscene].find(name);
      if (scr == nullptr)
        return std::string();
      newName = freeName(project.scripts[Script::Cutscene], scr->name);
      copyScript(project, Script::Cutscene, name, newName);
      break;
    }
    case ResourceID::Room:{
      const Room* room = project.rooms.find(name);
      if (room == nullptr)
        return std::string();
      Room copy = *room;
      newName = freeName(project.rooms, room->name);
      copy.name = newName;
      //the placed objects keep their names, with their scripts for the new room
      for (const ObjectInstance& obj : room->objects)
        copyScript(project, Script::Object, Script::objectScriptName(obj.name, room->name), Script::objectScriptName(obj.name, newName));
      copyScript(project, Script::Room, room->name, newName);
      forWalkmapScripts(*room, [&](int x, int y){
        copyScript(project, Script::Walkmap, Script::walkmapScriptName(x, y, room->name), Script::walkmapScriptName(x, y, newName));
      });
      std::string oldName = room->name;
      project.rooms.add(newName, copy);
      //the placed characters get names of their own, and their scripts
      std::vector<CharacterInstance> chars = project.roomCharacters(oldName);
      for (const CharacterInstance& chr : chars){
        CharacterInstance& inst = placeCharacter(project, newName, chr.character, chr.position);
        inst.lookDir = chr.lookDir;
        inst.unmovable = chr.unmovable;
        inst.locked = chr.locked;
        copyScript(project, Script::Character, chr.name, inst.name);
      }
      break;
    }
    default:
      return std::string();
  }
  addToPool(project, kind, newName, folder.empty() ? pathInPool(project.gamePool, kind, name) : folder);
  return newName;
}

bool renameResource(Project& project, ResourceID kind, const std::string& from, const std::string& to, std::string* error){
  auto fail = [&](const std::string& why){
    if (error)
      *error = why;
    return false;
  };
  if (to.empty())
    return fail("A name can't be empty.");
  bool taken = false;
  switch (kind){
    case ResourceID::Item: taken = takenByOther(project.items, from, to); break;
    case ResourceID::Object: taken = takenByOther(project.objects, from, to); break;
    case ResourceID::Character: taken = takenByOther(project.characters, from, to); break;
    case ResourceID::Room: taken = takenByOther(project.rooms, from, to); break;
    case ResourceID::Script: taken = takenByOther(project.scripts[Script::Cutscene], from, to); break;
    default: return fail("This can't be renamed.");
  }
  if (taken)
    return fail("There is one called " + toUtf8(to) + " already.");
  if (from == to)
    return true;
  switch (kind){
    case ResourceID::Item:
      if (!project.items.find(from))
        return fail("There is nothing called " + toUtf8(from) + ".");
      renameScript(project, Script::Item, from, to);
      renameEntry(project.items, from, to);
      break;
    case ResourceID::Object:
      if (!project.objects.find(from))
        return fail("There is nothing called " + toUtf8(from) + ".");
      renameEntry(project.objects, from, to);
      //the rooms refer to it by name
      for (auto& room : project.rooms){
        for (ObjectInstance& inst : room.value.objects){
          if (toLower(inst.object) == toLower(from))
            inst.object = to;
        }
      }
      break;
    case ResourceID::Character:
      if (!project.characters.find(from))
        return fail("There is nothing called " + toUtf8(from) + ".");
      renameEntry(project.characters, from, to);
      for (auto& room : project.characterInstances){
        for (CharacterInstance& inst : room.value){
          if (toLower(inst.character) == toLower(from))
            inst.character = to;
        }
      }
      break;
    case ResourceID::Script:
      if (!project.scripts[Script::Cutscene].find(from))
        return fail("There is nothing called " + toUtf8(from) + ".");
      renameScript(project, Script::Cutscene, from, to);
      break;
    case ResourceID::Room:{
      //the C# Room.rename: its script, its objects' and walkmap's scripts, its characters
      Room* room = project.rooms.find(from);
      if (room == nullptr)
        return fail("There is nothing called " + toUtf8(from) + ".");
      std::string oldName = room->name;
      renameScript(project, Script::Room, oldName, to);
      for (const ObjectInstance& obj : room->objects)
        renameScript(project, Script::Object, Script::objectScriptName(obj.name, oldName), Script::objectScriptName(obj.name, to));
      forWalkmapScripts(*room, [&](int x, int y){
        renameScript(project, Script::Walkmap, Script::walkmapScriptName(x, y, oldName), Script::walkmapScriptName(x, y, to));
      });
      if (std::vector<CharacterInstance>* chars = project.characterInstances.find(oldName)){
        std::vector<CharacterInstance> moved = *chars;
        for (CharacterInstance& chr : moved)
          chr.room = to;
        project.characterInstances.remove(oldName);
        project.characterInstances.set(to, moved);
      }
      for (FxShape& shape : room->fxShapes){
        if (toLower(shape.room) == toLower(oldName))
          shape.room = to;
      }
      renameEntry(project.rooms, oldName, to);
      break;
    }
    default:
      break;
  }
  renameInPool(project, kind, from, to);
  return true;
}

void deleteResource(Project& project, ResourceID kind, const std::string& name){
  switch (kind){
    case ResourceID::Image:
    case ResourceID::Sound:
    case ResourceID::Music:
    case ResourceID::Video:
      mediaFiles(project, kind)->remove(name);
      break;
    case ResourceID::Item:
      project.scripts[Script::Item].remove(name);
      project.items.remove(name);
      break;
    case ResourceID::Object:
      //its placed objects in all rooms, with their scripts
      for (auto& room : project.rooms){
        for (size_t i = room.value.objects.size(); i-- > 0;){
          if (toLower(room.value.objects[i].object) == toLower(name))
            removeObjectInstance(project, room.value, i);
        }
      }
      project.objects.remove(name);
      break;
    case ResourceID::Character:
      for (auto& room : project.characterInstances){
        std::vector<CharacterInstance>& chars = room.value;
        for (size_t i = chars.size(); i-- > 0;){
          if (toLower(chars[i].character) == toLower(name)){
            project.scripts[Script::Character].remove(chars[i].name);
            chars.erase(chars.begin()+i);
          }
        }
      }
      project.characters.remove(name);
      break;
    case ResourceID::Room:{
      Room* room = project.rooms.find(name);
      if (room == nullptr)
        break;
      std::string roomName = room->name;
      while (!room->objects.empty())
        removeObjectInstance(project, *room, 0);
      for (const CharacterInstance& chr : project.roomCharacters(roomName))
        project.scripts[Script::Character].remove(chr.name);
      project.characterInstances.remove(roomName);
      forWalkmapScripts(*room, [&](int x, int y){
        project.scripts[Script::Walkmap].remove(Script::walkmapScriptName(x, y, roomName));
      });
      project.scripts[Script::Room].remove(roomName);
      project.rooms.remove(roomName);
      break;
    }
    case ResourceID::Script:
      project.scripts[Script::Cutscene].remove(name);
      break;
    case ResourceID::Folder:
      return;
  }
  removeFromPool(poolOf(project, inGamePool(kind)), kind, name);
}

namespace{

void deleteNodes(Project& project, const PoolNode& node){
  if (node.id == (int)ResourceID::Folder){
    for (const PoolNode& child : node.children)
      deleteNodes(project, child);
  }
  else
    deleteResource(project, (ResourceID)node.id, node.label);
}

}

void deletePoolNode(Project& project, bool game, const PoolPath& path){
  //the top folders stay
  if (path.size() < 2)
    return;
  std::vector<PoolNode>& pool = poolOf(project, game);
  PoolNode* node = poolNode(pool, path);
  if (node == nullptr)
    return;
  PoolNode copy = *node;
  if (copy.id != (int)ResourceID::Folder){
    deleteResource(project, (ResourceID)copy.id, copy.label);
    return;
  }
  deleteNodes(project, copy);
  //the folder (its resources are gone from it already)
  PoolPath parentPath(path.begin(), path.end()-1);
  PoolNode* parent = poolNode(pool, parentPath);
  if (parent == nullptr)
    return;
  for (size_t i = 0; i < parent->children.size(); ++i){
    if (parent->children[i].id == (int)ResourceID::Folder && parent->children[i].label == copy.label){
      parent->children.erase(parent->children.begin()+i);
      break;
    }
  }
}

PoolPath newFolder(Project& project, bool game, const PoolPath& path){
  std::vector<PoolNode>& pool = poolOf(project, game);
  PoolPath parentPath = path;
  PoolNode* parent = poolNode(pool, parentPath);
  if (parent && parent->id != (int)ResourceID::Folder){
    parentPath.pop_back();
    parent = poolNode(pool, parentPath);
  }
  if (parent == nullptr)
    return PoolPath();
  int count = 0;
  for (const PoolNode& n : parent->children){
    if (n.id == (int)ResourceID::Folder)
      ++count;
  }
  std::string label = "Folder" + std::to_string(count+1);
  //a name of its own, which the C# one doesn't check
  auto taken = [&](const std::string& l){
    for (const PoolNode& n : parent->children){
      if (n.id == (int)ResourceID::Folder && n.label == l)
        return true;
    }
    return false;
  };
  while (taken(label))
    label = "Folder" + std::to_string(++count+1);
  parent->children.push_back(folderNode(label));
  sortPool(parent->children);
  for (size_t i = 0; i < parent->children.size(); ++i){
    if (parent->children[i].id == (int)ResourceID::Folder && parent->children[i].label == label){
      parentPath.push_back(i);
      return parentPath;
    }
  }
  return PoolPath();
}

bool renameFolder(Project& project, bool game, const PoolPath& path, const std::string& to){
  std::vector<PoolNode>& pool = poolOf(project, game);
  if (to.empty() || path.size() < 2)
    return false;
  std::vector<PoolNode>* siblings = parentList(pool, path);
  PoolNode* node = poolNode(pool, path);
  if (siblings == nullptr || node == nullptr || node->id != (int)ResourceID::Folder)
    return false;
  for (const PoolNode& n : *siblings){
    if (&n != node && n.label == to)
      return false;
  }
  node->label = to;
  sortPool(*siblings);
  return true;
}

bool movePoolNode(Project& project, bool game, const PoolPath& from, const PoolPath& toFolder){
  std::vector<PoolNode>& pool = poolOf(project, game);
  if (from.size() < 2 || toFolder.empty())
    return false;
  if (kindAt(pool, game, from) != kindAt(pool, game, toFolder))
    return false;
  //not into itself or below it, nor where it is
  if (toFolder.size() >= from.size() && std::equal(from.begin(), from.end(), toFolder.begin()))
    return false;
  if (toFolder == PoolPath(from.begin(), from.end()-1))
    return false;
  PoolNode* target = poolNode(pool, toFolder);
  std::vector<PoolNode>* source = parentList(pool, from);
  if (target == nullptr || target->id != (int)ResourceID::Folder || source == nullptr || from.back() >= source->size())
    return false;
  PoolNode moved = (*source)[from.back()];
  //removing it may move the target's place among its siblings: find it again by its label path
  std::vector<std::string> labels;
  std::vector<PoolNode>* level = &pool;
  for (size_t idx : toFolder){
    labels.push_back((*level)[idx].label);
    level = &(*level)[idx].children;
  }
  source->erase(source->begin()+from.back());
  level = &pool;
  PoolNode* dest = nullptr;
  for (const std::string& label : labels){
    dest = nullptr;
    for (PoolNode& n : *level){
      if (n.id == (int)ResourceID::Folder && n.label == label){
        dest = &n;
        break;
      }
    }
    if (dest == nullptr)
      return false;
    level = &dest->children;
  }
  dest->children.push_back(moved);
  sortPool(dest->children);
  return true;
}

NamedList<std::string>* mediaFiles(Project& project, ResourceID kind){
  switch (kind){
    case ResourceID::Image: return &project.images;
    case ResourceID::Sound: return &project.sounds;
    case ResourceID::Music: return &project.music;
    case ResourceID::Video: return &project.videos;
    default: return nullptr;
  }
}

const NamedList<std::string>* mediaFiles(const Project& project, ResourceID kind){
  return mediaFiles(const_cast<Project&>(project), kind);
}

bool addMedia(Project& project, ResourceID kind, const std::string& file, const PoolPath& folder, std::string* error){
  NamedList<std::string>* files = mediaFiles(project, kind);
  if (files == nullptr)
    return false;
  std::string name = fileNameOf(file);
  size_t dot = name.find_last_of('.');
  if (dot != std::string::npos && dot > 0)
    name = name.substr(0, dot);
  if (!files->add(name, file)){
    if (error)
      *error = "There is a file called " + toUtf8(name) + " already.";
    return false;
  }
  addToPool(project, kind, name, folder);
  return true;
}

namespace{

using Missing = std::map<std::pair<int, std::string>, std::string>;

bool resolveIn(Project& project, const fs::path& dir, Missing& missing){
  std::error_code ec;
  std::vector<fs::path> files, dirs;
  for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)){
    if (it->is_directory(ec))
      dirs.push_back(it->path());
    else
      files.push_back(it->path());
  }
  //the subfolders first, as the C# one
  std::sort(dirs.begin(), dirs.end());
  for (const fs::path& sub : dirs){
    if (resolveIn(project, sub, missing))
      return true;
  }
  for (const fs::path& file : files){
    std::u8string u8 = file.filename().u8string();
    std::string name = fromUtf8(std::string(u8.begin(), u8.end()));
    for (auto it = missing.begin(); it != missing.end();){
      if (it->second == name){
        std::u8string full = file.u8string();
        mediaFiles(project, (ResourceID)it->first.first)->set(it->first.second, fromUtf8(std::string(full.begin(), full.end())));
        it = missing.erase(it);
      }
      else
        ++it;
    }
  }
  return missing.empty();
}

fs::path toPath(const std::string& cp1252){
  std::string utf8 = toUtf8(cp1252);
  return fs::path(std::u8string(utf8.begin(), utf8.end()));
}

}

bool resolveMedia(Project& project, const std::string& folder){
  Missing missing;
  for (ResourceID kind : {ResourceID::Image, ResourceID::Sound, ResourceID::Music, ResourceID::Video}){
    for (const auto& entry : *mediaFiles(project, kind)){
      std::error_code ec;
      if (!fs::is_regular_file(toPath(entry.value), ec))
        missing[{(int)kind, entry.key}] = fileNameOf(entry.value);
    }
  }
  if (missing.empty())
    return true;
  return resolveIn(project, toPath(folder), missing);
}

}
