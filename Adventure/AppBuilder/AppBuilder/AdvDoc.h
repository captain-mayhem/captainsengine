#ifndef ADV_DOC_H
#define ADV_DOC_H

#include <map>
#include <vector>

#include <wx/docview.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/image.h>
#include "AdvMainTree.h"
#include "Vector.h"

class SoundPlayer;

struct ProjectSettings{
  std::string dir;
  std::string savedir;
  Vec2i resolution;
  //TODO fonts
  std::string startscript;
  std::string mainscript;
  std::string anywhere_room;
  ScreenChange screenchange;
  int taskheight;
  std::string taskroom;
  int taskpopup;
  bool taskHideCompletely;
  int targacolor;
  int bordercolor;
  int backgroundcolor;
  int textcolor;
  int offtextcolor;
  TsStyle tsstyle;
  int tsborderstyle;
  int tsareacolor;
  int tsselectioncolor;
  int tstextcolor;
  int tsbordercolor;
  std::string linktext;
  std::string givelink;
  std::string walktext;
  std::map<std::string,bool> booleans;
  std::map<std::string, unsigned> commands;
  std::vector<std::string> pretty_commands;

};


typedef std::vector<std::string> Frames;

struct CursorState{
  Frames frames;
  float fps;
  int command;
  Vec2i highlight;
};

typedef std::vector<CursorState> MouseCursor;

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
  Vec2i parallaxpoint;
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
  Vec2i fontsize;
  Vec2i numChars;
  std::vector<wxImage> images;
  std::vector<unsigned> charwidths;
};

class AdvDocument : public wxDocument{
public:
  AdvDocument();
  virtual ~AdvDocument();
  virtual wxOutputStream& SaveObject(wxOutputStream& stream);
  virtual wxInputStream& LoadObject(wxInputStream& stream);
  wxImage getImage(wxString name);
  SoundPlayer* getSound(const std::string& name);
  Room* getRoom(std::string name);
  Object* getObject(std::string name);
  MouseCursor* getCursor();
  std::vector<Rcharacter> getRoomCharacters() {return mRoomCharacters;}
  Character* getCharacter(std::string name);
  ProjectSettings* getProjectSettings();
  Script* getScript(Script::Type t, std::string name);
  std::vector<std::pair<Vec2i,Script*> > getWMScripts(std::string roomname);
  FontData getFont(int num);
  Item* getItem(const std::string& name);
protected:
  DECLARE_DYNAMIC_CLASS(AdvDocument)
  bool loadFile1(wxInputStream& stream);
  bool loadFile2(wxInputStream& stream);
  bool loadFile3(wxInputStream& stream);
  static int insertTreeElement(wxTreeCtrl* tree, const wxString& name, wxTreeItemId* current, int curr_level);
  float readExtendedFrames(wxTextInputStream& txtstream, ExtendedFrames& frms);
  //void loadFonts();
  AdvMainTreeView* mView;
  ProjectSettings mSettings;
  std::map<wxString,wxFileName> mImageNames;
  std::map<wxString,wxFileName> mSoundNames;
  std::map<wxString,wxFileName> mMusicNames;
  std::map<wxString,wxFileName> mMovieNames;
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
  wxFileSystem* mStream;
  //std::vector<Font> mFonts;
};

#endif
