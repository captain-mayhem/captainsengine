#ifndef ADV_TREE_CTRL
#define ADV_TREE_CTRL

#include <wx/treectrl.h>

enum ResourceID{
  SCRIPT=0,
  ITEM=1,
  MUSIC=2,
  OBJECT=3,
  IMAGE=4,
  ROOM=5,
  SOUND=6,
  FOLDER=11,
  CHARACTER=15,
};

class AdvTreeItemData : public wxTreeItemData{
public:
  AdvTreeItemData(ResourceID id) {mID = id;}
  virtual ~AdvTreeItemData() {}
  ResourceID getID() {return mID;}
protected:
  ResourceID mID;
};

class AdvTreeCtrl : public wxTreeCtrl{
public:
  AdvTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxTR_HAS_BUTTONS);
  virtual ~AdvTreeCtrl();
  virtual void OnItemActivated(wxTreeEvent& event);
  virtual void OnItemMenu(wxTreeEvent& event);
protected:
  DECLARE_EVENT_TABLE();
};

#endif
