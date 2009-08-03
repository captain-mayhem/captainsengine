#ifndef ADV_MAIN_TREE_VIEW
#define ADV_MAIN_TREE_VIEW

#include <wx/docview.h>
#include <wx/docmdi.h>
#include <wx/laywin.h>
#include "Graph.h"
#include "AdvTreeCtrl.h"

class AdvMainTreeView : public wxView{
public:
  AdvMainTreeView();
  virtual ~AdvMainTreeView();
  virtual bool OnCreate(wxDocument* doc, long flags);
  virtual bool OnClose(bool deleteWindow);
  virtual void OnDraw(wxDC* dc);
  wxTreeCtrl* getGamepool() {return mGamepool;}
  wxTreeCtrl* getMediapool() {return mMediapool;}
protected:
  static void buildTreeFromGraph(wxTreeCtrl* ctrl, const Graph& grf);
  wxDocMDIChildFrame* mFrame;
  wxMDIChildFrame* mGamepoolWindow;
  wxMDIChildFrame* mMediapoolWindow;
  AdvTreeCtrl* mGamepool;
  AdvTreeCtrl* mMediapool;
  DECLARE_DYNAMIC_CLASS(AdvMainTreeView)
};

#endif
