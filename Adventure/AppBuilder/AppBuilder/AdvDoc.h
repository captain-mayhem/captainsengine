#ifndef ADV_DOC_H
#define ADV_DOC_H

#include <map>
#include <vector>

#include <wx/docview.h>
#include <wx/filename.h>
#include "AdvMainTree.h"
//#include "Graph.h"

struct CursorState{
  std::vector<std::string> frames;
  int command;
  std::pair<int,int> highlight;
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
  std::vector<CursorState> mCursor;
  //Graph mGamepool;
};

#endif
