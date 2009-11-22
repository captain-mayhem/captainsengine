#ifndef ADV_DOC_H
#define ADV_DOC_H

#include <map>
#include <vector>

#include <wx/docview.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/image.h>
#include "AdvMainTree.h"
//#include "Graph.h"

struct Vec2i{
  Vec2i() : x(0), y(0) {}
  Vec2i(int x, int y) {this->x = x; this->y = y;}
  Vec2i operator+(const Vec2i& v) const{
    Vec2i result;
    result.x = v.x+x;
    result.y = v.y+y;
    return result;
  }
  Vec2i operator-(const Vec2i& v) const{
    Vec2i result;
    result.x = x-v.x;
    result.y = y-v.y;
    return result;
  }
  Vec2i operator*(int num) const{
    Vec2i result;
    result.x = x*num;
    result.y = y*num;
    return result;
  }
  Vec2i operator/(int num) const{
    Vec2i result;
    result.x = x/num;
    result.y = y/num;
    return result;
  }
  float length(){
    return sqrt((float)x*x+y*y);
  }
  bool operator<(const Vec2i& v) const{
    if (x == v.x)
      return y < v.y;
    return x < v.x;
  }
  bool operator>(const Vec2i& v) const{
    if (x == v.x)
      return y > v.y;
    return x > v.x;
  }
  bool operator==(const Vec2i& v) const{
    return x==v.x && y==v.y;
  }
  bool operator!=(const Vec2i& v) const{
    return x!=v.x || y!=v.y;
  }
  int x;
  int y;
};

struct Vec2f{
  Vec2f() : x(0), y(0) {}
  Vec2f(float x, float y) {this->x = x; this->y = y;}
  float x;
  float y;
};

struct ProjectSettings{
  std::string dir;
  Vec2i resolution;
  //TODO fonts
  std::string startscript;
  std::string mainscript;
  std::string anywhere_room;
  int screenchange;
  int targacolor;
  int bordercolor;
  int backgroundcolor;
  int textcolor;
  int offtextcolor;
  int areacolor;
  int selectioncolor;
  int textscenetextcolor;
  int textscenebordercolor;
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
