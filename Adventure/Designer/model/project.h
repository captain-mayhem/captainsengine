#ifndef SD_PROJECT_H
#define SD_PROJECT_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

// The StoryDesigner's model of an Adventure project: everything a project file
// (.adv, or a game's game.dat) holds, kept as it was read so that saving
// writes what the C# StoryDesigner wrote. Texts are bytes in Windows-1252, the
// files' encoding. Names are looked up without case, as in the files' users.

namespace sd{

struct Vec2i{
  int x = 0;
  int y = 0;
  bool operator==(const Vec2i& v) const {return x == v.x && y == v.y;}
};

struct Vec2f{
  float x = 0;
  float y = 0;
};

//! the lower case of a Windows-1252 text (as .NET's ToLower for it)
std::string toLower(const std::string& text);
//! a Windows-1252 text in UTF-8, for showing it
std::string toUtf8(const std::string& text);
//! a UTF-8 text in Windows-1252; characters it lacks become '?'
std::string fromUtf8(const std::string& text);

//! entries by a case-insensitive name, in the order they were added
/*! The order is that of the C# StoryDesigner's dictionaries, which a saved
    file keeps. */
template<typename T>
class NamedList{
public:
  //! false if the name is taken
  bool add(const std::string& name, const T& value){
    std::string key = toLower(name);
    if (index_.count(key) > 0)
      return false;
    index_[key] = entries_.size();
    entries_.push_back(Entry{key, value});
    return true;
  }
  //! adds or replaces
  void set(const std::string& name, const T& value){
    T* existing = find(name);
    if (existing)
      *existing = value;
    else
      add(name, value);
  }
  T* find(const std::string& name){
    auto iter = index_.find(toLower(name));
    return iter == index_.end() ? nullptr : &entries_[iter->second].value;
  }
  const T* find(const std::string& name) const{
    auto iter = index_.find(toLower(name));
    return iter == index_.end() ? nullptr : &entries_[iter->second].value;
  }
  bool remove(const std::string& name){
    auto iter = index_.find(toLower(name));
    if (iter == index_.end())
      return false;
    entries_.erase(entries_.begin() + iter->second);
    reindex();
    return true;
  }
  size_t size() const {return entries_.size();}
  bool empty() const {return entries_.empty();}
  void clear() {entries_.clear(); index_.clear();}

  struct Entry{
    //! the name in lower case
    std::string key;
    T value;
  };
  typename std::vector<Entry>::iterator begin() {return entries_.begin();}
  typename std::vector<Entry>::iterator end() {return entries_.end();}
  typename std::vector<Entry>::const_iterator begin() const {return entries_.begin();}
  typename std::vector<Entry>::const_iterator end() const {return entries_.end();}
private:
  void reindex(){
    index_.clear();
    for (size_t i = 0; i < entries_.size(); ++i)
      index_[entries_[i].key] = i;
  }
  std::vector<Entry> entries_;
  std::map<std::string, size_t> index_;
};

//! a resource's kind in the media and game pools (the numbers are the files')
enum class ResourceID{
  Script = 0,
  Item = 1,
  Music = 2,
  Object = 3,
  Image = 4,
  Room = 5,
  Sound = 6,
  Folder = 11,
  Character = 15,
  Video = 20,
};

//! an entry of the media pool or the game pool: a resource or a folder
struct PoolNode{
  int id = 0;
  std::string label;
  std::vector<PoolNode> children;
};

enum class ScreenChange{
  Direct = 0,
  Fadeout,
  Rectangle,
  Circle,
  Shutters,
  Clock,
  Blend,
  BlendSlow,
};

struct FontInfo{
  std::string name;
  int charset = 0;
  bool bold = false;
  bool italic = false;
  int size = 0;
  int outline = 0;
  int shadow = 0;
  int fill = 0;
  int fading = 0;
  int spacing = 0;
};

struct DSPEffect{
  enum Type{
    Reverb = 1,
    Echo,
    Chorus,
    Distortion,
    Phaser,
  };
  std::string name;
  int type = 0;
  int param[6][6] = {};
};

enum class ScriptLang{
  PCDK = 0,
  Lua = 1,
};

//! the project's settings, with the C# StoryDesigner's defaults for a new project
struct ProjectSettings{
  std::string projectName = "New Project";
  std::string directory;
  Vec2i resolution{800, 600};
  std::string gameIcon;
  std::string loadingImage;
  std::string settingsPicture;
  bool tsUseBgImage = false;
  std::string tsBackground;
  std::string startScript;
  std::string mainScript;
  std::string anywhereRoom;
  ScreenChange screenChange = ScreenChange::Direct;
  bool textOnOff = false;
  bool drawDraggedItemIcons = false;
  bool actionText = true;
  bool showTaskbar = true;
  bool taskbarFromTop = false;
  bool notAntialiased = false;
  bool groupItems = false;
  int actionTextHeight = 0;
  bool customMenu = false;
  std::string customMenuRoom;
  int taskHeight = 140;
  std::string taskRoom;
  int taskPopup = 0;
  bool silentDelete = false;
  bool infoLine = true;
  bool protectGameFile = false;
  bool is16to9 = false;
  int menuFading = 0;
  int textSceneFading = 0;
  bool taskHideCompletely = true;
  int anywhereTransparency = 0;
  uint32_t targaColor = 16711935;
  uint32_t borderColor = 8421376;
  uint32_t backgroundColor = 32768;
  uint32_t textColor = 65280;
  uint32_t offspeechColor = 16777215;
  uint32_t infotextColor = 16777215;
  int tsStyle = 0;
  int tsBorderStyle = 0;
  uint32_t tsBorderColor = 0;
  uint32_t tsAreaColor = 8421504;
  uint32_t tsSelectionColor = 12632256;
  uint32_t tsTextColor = 15793151;
  bool tsUseSymbols = false;
  bool muteMusicWhenSpeech = true;
  bool coinActivated = false;
  bool coinAutoPopup = true;
  std::string coinRoom;
  int coinFading = 0;
  Vec2i coinCenter{90, 65};
  std::vector<DSPEffect> dspEffects;
  std::string linkText = "with";
  std::string giveLink = "to";
  std::string walkText = "Walk to";
  //! the boolean variables and their start values
  std::vector<std::pair<std::string, bool>> booleans;
  //! the commands (name, shown name)
  std::vector<std::pair<std::string, std::string>> commands;
  int rightClick = 0;
  bool useMouseWheel = false;
  std::vector<FontInfo> fonts;
  bool pngToJpeg = false;
  ScriptLang scriptingLanguage = ScriptLang::PCDK;
};

struct CursorState{
  std::vector<std::string> frames;
  int fpsDivider = 20;
  //! the command, counted from 1 as in the file
  int command = 1;
  Vec2i highlight{1, 1};
  Vec2i itemOffset{16, 16};
};

struct ItemState{
  //! the frames' images; a frame's script is at the same index
  std::vector<std::string> frames;
  std::vector<std::string> scripts;
  int fpsDivider = 20;
};

struct Item{
  std::string name;
  std::vector<ItemState> states;
};

//! a frame of up to two image parts with their offsets, and a script
struct ExtendedFrame{
  std::vector<std::string> names;
  std::vector<Vec2i> offsets;
  std::string script;
};

struct ObjectState{
  std::vector<ExtendedFrame> frames;
  int fpsDivider = 20;
};

struct Object{
  std::string name;
  Vec2i size;
  bool lighten = false;
  std::vector<ObjectState> states;
};

struct CharacterState{
  Vec2i size{120, 200};
  Vec2i basepoint{60, 199};
  std::vector<ExtendedFrame> frames;
  int fpsDivider = 20;
};

struct Character{
  std::string name;
  uint32_t textColor = 0;
  int walkSpeed = 0;
  bool noZoom = false;
  bool realLeftAnimations = false;
  bool memoryResistant = false;
  bool ghost = false;
  std::string walkSound;
  //! the names of the extra states 17 to 36
  std::vector<std::string> extraStateNames;
  int font = 0;
  int zoom = 0;
  std::vector<CharacterState> states;
};

//! a character placed in a room
struct CharacterInstance{
  std::string name;
  std::string character;
  std::string room;
  Vec2i position;
  int lookDir = 0;
  bool unmovable = false;
  bool locked = false;
};

struct FxShape{
  enum Effect{
    FloorMirror = 0,
    WallMirror = 1,
    ParticleBarrier = 2,
  };
  bool active = false;
  bool dependingOnRoomPosition = false;
  int effect = FloorMirror;
  std::string room;
  int depth = 0;
  Vec2i mirrorOffset;
  int strength = 0;
  Vec2i positions[4];
  //! the default of a new room's shape number num
  static FxShape makeDefault(int num);
};

//! an object placed in a room
struct ObjectInstance{
  std::string name;
  std::string object;
  Vec2i position;
  int state = 0;
  int layer = 0;
  int depth = 0;
  bool locked = false;
};

struct WalkMapEntry{
  bool isFree = false;
  bool hasScript = false;
};

struct Room{
  std::string name;
  Vec2i size;
  Vec2i scrollOffset;
  Vec2i depthmap;
  int zoom = 0;
  std::string background;
  std::string parallaxBackground;
  bool doubleWalkmap = false;
  std::vector<FxShape> fxShapes;
  bool hasInventory = false;
  Vec2i invPos;
  Vec2i invSize;
  Vec2f invScale;
  int invSpacing = 10;
  //! [x][y]
  std::vector<std::vector<WalkMapEntry>> walkmap;
  std::vector<ObjectInstance> objects;
  uint8_t lighting[3] = {255, 255, 255};
};

struct Script{
  enum Type{
    Cutscene,
    Item,
    Character,
    Room,
    Object,
    Walkmap,
    NumTypes
  };
  Type type = Cutscene;
  std::string name;
  //! empty for a script whose name has no text after it
  std::optional<std::string> text;

  //! the name of an object's script in a room
  static std::string objectScriptName(const std::string& object, const std::string& room);
  //! the name of a walkmap field's script (x, y from 0)
  static std::string walkmapScriptName(int x, int y, const std::string& room);
};

struct Language{
  enum Section{
    Speech = 0,
    SpeechSounds,
    Offspeech,
    OffspeechSounds,
    Showinfo,
    Textout,
    Setstring,
    Textscenes,
    Commands,
    NumSections
  };
  std::string name;
  std::vector<std::string> sections[NumSections];
};

//! an Adventure project
class Project{
public:
  Project();

  ProjectSettings settings;
  std::vector<PoolNode> mediaPool;
  std::vector<PoolNode> gamePool;
  //! name to file
  NamedList<std::string> images;
  NamedList<std::string> sounds;
  NamedList<std::string> music;
  NamedList<std::string> videos;
  std::vector<CursorState> cursor;
  NamedList<Item> items;
  NamedList<Object> objects;
  NamedList<Character> characters;
  //! the characters placed in rooms, by room
  NamedList<std::vector<CharacterInstance>> characterInstances;
  NamedList<Room> rooms;
  NamedList<Script> scripts[Script::NumTypes];
  NamedList<Language> languages;
  //! the password of the game's data files (empty: none)
  std::string zipPassword;
  //! the version of the file the project was read from
  int versionMajor = 3;
  int versionMinor = 6;

  //! the size of the walkmap's fields in pixels
  int walkGridSize() const {return settings.resolution.x / 32;}
  //! the size of a room's walkmap in fields
  Vec2i walkmapSize() const;
  //! the characters placed in a room (empty if none)
  const std::vector<CharacterInstance>& roomCharacters(const std::string& room) const;
};

}

#endif
