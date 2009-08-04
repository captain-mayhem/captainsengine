#ifndef ADV_DOC_H
#define ADV_DOC_H

#include <map>
#include <vector>

#include <wx/docview.h>
#include <wx/filename.h>
#include "AdvMainTree.h"
//#include "Graph.h"

typedef std::vector<std::string> Frames;

struct CursorState{
  Frames frames;
  int command;
  std::pair<int,int> highlight;
};

typedef std::vector<CursorState> Cursor;

struct Item{
  std::string name;
  std::vector<Frames> frames;
};

struct ExtendedFrame{
  std::vector<std::string> names;
  std::vector<std::pair<int,int> > offsets;
};

typedef std::vector<ExtendedFrame> ExtendedFrames;

struct Object{
  std::string name;
  std::pair<int,int> size;
  int unknown;
  std::vector<ExtendedFrames> frames;
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
  int unknown;
  int zoom;
  std::pair<int,int> size;
  std::pair<int,int> basepoint;
  std::vector<ExtendedFrames> frames;
};

class AdvDocument : public wxDocument{
public:
  AdvDocument();
  virtual ~AdvDocument();
  virtual wxOutputStream& SaveObject(wxOutputStream& stream);
  virtual wxInputStream& LoadObject(wxInputStream& stream);
  wxFileName getFilename(ResourceID id, wxString name);
  //Graph& getGamepool() {return mGamepool;}
protected:
  DECLARE_DYNAMIC_CLASS(AdvDocument)
  bool loadFile1(wxInputStream& stream);
  bool loadFile2(wxInputStream& stream);
  static int insertTreeElement(wxTreeCtrl* tree, const wxString& name, wxTreeItemId* current, int curr_level);
  AdvMainTreeView* mView;
  std::map<wxString,wxFileName> mImageNames;
  Cursor mCursor;
  std::vector<Item> mItems;
  std::vector<Object> mObjects;
  //Graph mGamepool;
};

#endif
