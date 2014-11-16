#ifndef ADV_DOC_H
#define ADV_DOC_H

#include <map>
#include <vector>
#include <cstring>

#include <image/image.h>
#include <system/types.h>
#include <system/thread.h>
#include "Ids.h"
#include "Vector.h"
#include "String.h"

namespace CGE{
  class MemReader;
}

namespace adv{

class SoundPlayer;

struct ProjectSettings{
  std::string dir;
  std::string savedir;
  Vec2i resolution;
  std::vector<int> font_fading;
  std::string startscript;
  std::string mainscript;
  std::string anywhere_room;
  ScreenChange screenchange;
  bool draw_dragged_items;
  bool show_actiontext;
  bool show_taskbar;
  bool group_items;
  bool has_menuroom;
  std::string menuroom;
  int taskheight;
  std::string taskroom;
  int taskpopup;
  bool taskHideCompletely;
  int right_click;
  int menu_fading;
  int textscene_fading;
  int anywhere_transparency;
  int targacolor;
  int bordercolor;
  int backgroundcolor;
  int textcolor;
  int offspeechcolor;
  int infotextcolor;
  TsStyle tsstyle;
  int tsborderstyle;
  int tsareacolor;
  int tsselectioncolor;
  int tstextcolor;
  int tsbordercolor;
  std::string splashscreen;
  std::string tsbackground;
  std::string linktext;
  std::string givelink;
  std::string walktext;
  std::map<String,bool> booleans;
  std::map<std::string, unsigned> commands;
  std::vector<std::string> pretty_commands;

  bool noPngToJpeg;
};


typedef std::vector<std::string> SimpleFrames;

struct CursorState{
  SimpleFrames frames;
  float fps;
  int command;
  Vec2i highlight;
};

typedef std::vector<CursorState> MouseCursor;

struct Frame{
  std::string name;
  std::string script;
};

typedef std::vector<Frame> Frames;

struct ItemState{
  Frames frames;
  float fps;
};

struct Item{
  std::string name;
  std::vector<ItemState> states;
};

struct ExtendedFrame{
  std::vector<std::string> names;
  std::vector<Vec2i> offsets;
  std::string script;
};

typedef std::vector<ExtendedFrame> ExtendedFrames;

struct ObjectState{
  ExtendedFrames frames;
  float fps;
};

struct Object{
  std::string name;
  Vec2i size;
  bool lighten;
  std::vector<ObjectState> states;
};

struct CharacterState{
  Vec2i size;
  Vec2i basepoint;
  ExtendedFrames frames;
  float fps;
};

struct Character{
  std::string name;
  int textcolor;
  int walkspeed;
  bool notzoom;
  bool realleft;
  bool memresistent;
  bool ghost;
  std::string walksound;
  std::vector<std::string> extrastatenames;
  int fontid;
  int zoom;
  std::vector<CharacterState> states;
};

struct Rcharacter{
  std::string name;
  std::string character;
  std::string room;
  Vec2i position;
  LookDir dir;
  int unknown;
  bool unmovable;
  bool locked;
};

struct FXShape{
  enum FxEffect{
    FLOOR_MIRROR = 0,
    WALL_MIRROR = 1,
  PARTICLE_BARRIER = 2,
  };
  bool active;
  bool dependingOnRoomPosition;
  FxEffect effect;
  std::string room;
  int depth;
  Vec2i mirrorOffset;
  int strength;
  Vec2i points[4];
};

struct WMField{
  bool walkable;
  bool script;
};

struct Roomobject{
  std::string name;
  std::string object;
  Vec2i position;
  int state;
  int layer;
  int wm_depth;
  bool locked;
};

struct Room{
  std::string name;
  Vec2i size;
  Vec2i scrolloffset;
  Vec2i depthmap;
  int zoom;
  std::string background;
  std::string parallaxbackground;
  bool doublewalkmap;
  std::vector<FXShape> fxshapes;
  bool hasInventory;
  Vec2i invpos;
  Vec2i invsize;
  Vec2f invscale;
  std::vector<std::vector<WMField> > walkmap;
  std::vector<Roomobject> objects;
};

struct Script{
  enum Type{
    CUTSCENE,
    ITEM,
    CHARACTER,
    ROOM,
    OBJECT,
    WALKMAP,
  };
  Script() {}
  ~Script() {}
  Script(const Script& s){
    type = s.type;
    name = s.name;
    text = s.text;
  }
  Type type;
  std::string name;
  std::string text;
};

struct FontData{
  void destroyImages(){
    for (unsigned i = 0; i < images.size(); ++i){
      delete images[i];
    }
  }
  Vec2i fontsize;
  Vec2i numChars;
  std::vector<CGE::Image*> images;
  std::vector<unsigned> charwidths;
};

struct DataBuffer{
  DataBuffer() : data(NULL), length(0), used(0) {}
  DataBuffer(const DataBuffer& d){
    name = d.name;
    data = new char[d.length];
    memcpy(data, d.data, d.length);
    length = d.length;
    used = d.used;
  }
  ~DataBuffer(){
    delete [] data;
  }
  std::string name;
  char* data;
  unsigned length;
  unsigned used;
};

struct Language{
  enum Section{
    SPEECH=0,
    SPEECH_SOUNDS,
    OFFSPEECH,
    OFFSPEECH_SOUNDS,
    SHOWINFO,
    TEXTOUT,
    SETSTRING,
    TEXTSCENES,
    COMMANDS,
    NUM_SECTIONS
  };
  std::vector<std::string> sections[NUM_SECTIONS];
};

typedef void(*SPLASHSCREENCB)(unsigned width, unsigned height, unsigned numChannels, void* data);

class AdvDocument{
public:
  AdvDocument();
  virtual ~AdvDocument();
  //virtual wxOutputStream& SaveObject(wxOutputStream& stream);
  //virtual wxInputStream& LoadObject(wxInputStream& stream);
  bool loadDocument(const std::string filename);
  CGE::Image* getImage(const std::string& name);
  bool getSound(const std::string& name, DataBuffer& db);
  bool getMusic(const std::string& name, DataBuffer& db);
  bool getMovie(const std::string& name, DataBuffer& db);
  Room* getRoom(std::string name);
  Object* getObject(std::string name);
  Roomobject* findRoomObject(const std::string& name, Room*& containingRoom);
  MouseCursor* getCursor();
  std::vector<Rcharacter> getRoomCharacters() {return mRoomCharacters;}
  Character* getCharacter(std::string name);
  ProjectSettings* getProjectSettings();
  Script* getScript(Script::Type t, std::string name);
  std::vector<std::pair<Vec2i,Script*> > getWMScripts(std::string roomname);
  FontData getFont(int num);
  Item* getItem(const std::string& name);
  Language::Section getLanguageSection(const String& funcname, int argnum);
  std::string getLanguageString(const std::string& language, Language::Section section, int strindex);
  int getLanguageIndex(const std::string& language, Language::Section section, const std::string& str);
  bool hasLanguageInfo() {return mLanguages.size() > 0 && mLanguages["origin"].sections[Language::COMMANDS].size() > 0;}
  void setSpashScreenCB(SPLASHSCREENCB sscb) { mSSCB = sscb; }
protected:
  bool loadFile1(CGE::MemReader& txtstream);
  bool loadFile2(CGE::MemReader& txtstream);
  bool loadFile3(CGE::MemReader& txtstream);
  bool loadFile4(CGE::MemReader& txtstream);
  bool loadFile5(CGE::MemReader& txtstream);
  bool loadFile10(CGE::MemReader& txtstream);
  float readExtendedFrames(CGE::MemReader& txtstream, ExtendedFrames& frms);
  std::string animationScript(const std::string& input);
  ProjectSettings mSettings;
  std::map<std::string,std::string> mImageNames;
  std::map<std::string,std::string> mSoundNames;
  std::map<std::string,std::string> mMusicNames;
  std::map<std::string,std::string> mMovieNames;
  MouseCursor mCursor;
  std::map<std::string,Item> mItems;
  std::map<std::string,Object> mObjects;
  std::map<std::string,Character> mCharacters;
  std::vector<Rcharacter> mRoomCharacters;
  std::map<std::string,Room> mRooms;
  std::map<std::pair<Script::Type,std::string>,Script> mScripts;
  std::map<std::string, std::vector<std::pair<Vec2i,Script> > > mWMScripts;
  Room* mLastRoom;
  Script* mLastScript;
  std::string mPath;
  std::string mFilename;
  bool mUseCompressedData;
  std::string mZipPwd;
  std::map<std::string, Language> mLanguages;
  CGE::Mutex mMuty;
  SPLASHSCREENCB mSSCB;
};

}

#endif
