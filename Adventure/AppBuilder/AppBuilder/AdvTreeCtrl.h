#ifndef ADV_TREE_CTRL
#define ADV_TREE_CTRL

#include <wx/treectrl.h>
#include "Ids.h"

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
