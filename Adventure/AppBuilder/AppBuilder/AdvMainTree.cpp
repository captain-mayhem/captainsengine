
#include "AdvMainTree.h"
#include <wx/dc.h>
#include "main.h"
#include "AdvDoc.h"
#include "GraphNodes.h"
#include "Ids.h"
#include "Engine.h"
#include "Sound.h"

IMPLEMENT_DYNAMIC_CLASS(AdvMainTreeView, wxView);

AdvMainTreeView::AdvMainTreeView(){

}

AdvMainTreeView::~AdvMainTreeView(){
}

bool AdvMainTreeView::OnCreate(wxDocument* doc, long flags){
  mFrame = new wxDocMDIChildFrame(doc, this, wxGetApp().getFrame(), wxID_ANY,
    _T("Child frame"), wxPoint(10,10), wxSize(300,300), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
  mFrame->Show(true);
  mGamepoolWindow = new wxMDIChildFrame(wxGetApp().getFrame(), wxID_ANY, _T("Gamepool"), wxPoint(0,0), wxSize(200,200), wxSW_3D|wxCLIP_CHILDREN);
  mMediapoolWindow = new wxMDIChildFrame(wxGetApp().getFrame(), wxID_ANY, _T("Mediapool"), wxPoint(0,200), wxSize(200,200), wxSW_3D|wxCLIP_CHILDREN);
  mGamepool = new AdvTreeCtrl(mGamepoolWindow, ID_AdvTreeCtrl, wxDefaultPosition, wxSize(200,200), wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT);
  mMediapool = new AdvTreeCtrl(mMediapoolWindow, ID_AdvTreeCtrl, wxDefaultPosition, wxSize(200,200), wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_HIDE_ROOT);
  AdvDocument* adv = wxDynamicCast(doc, AdvDocument);
  Engine::instance()->setData(adv);
  SoundEngine::instance()->setData(adv);
  Activate(true);
  return true;
}

bool AdvMainTreeView::OnClose(bool deleteWindow){
  if (!GetDocument()->Close())
    return false;
  Activate(false);
  if(deleteWindow){
    delete mFrame;
  }
  return true;
}

void AdvMainTreeView::OnDraw(wxDC* dc){
  dc->DrawText(_T("Test"),wxPoint(10,10));
}

void AdvMainTreeView::buildTreeFromGraph(wxTreeCtrl* ctrl, const Graph& grf){
  TextNode* node = static_cast<TextNode*>(grf.getRoot());
  wxTreeItemId id = ctrl->AddRoot(node->getLabel().c_str());
}

