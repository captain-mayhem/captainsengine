
#include "AdvTreeCtrl.h"
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/image.h>
#include "Ids.h"
#include "main.h"
#include "ImageDisplay.h"
#include "AdvDoc.h"


AdvTreeCtrl::AdvTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxTreeCtrl(parent,id,pos,size,style){
}

AdvTreeCtrl::~AdvTreeCtrl(){

}

void AdvTreeCtrl::OnItemActivated(wxTreeEvent& event){
  wxTreeItemId id = event.GetItem();
  wxString name = GetItemText(id);
  AdvTreeItemData *item = (AdvTreeItemData *)GetItemData(id);
  if (item == NULL)
    return;
  switch (item->getID()){
    case FOLDER:
      if (IsExpanded(id))
        Collapse(id);
      else
        Expand(id);
      break;
    case IMAGE:{
      AdvDocument* doc = static_cast<AdvDocument*>(wxGetApp().getDocument());
      wxImage image = doc->getImage(name.c_str());
      int width, height;
      width = image.GetWidth();
      height = image.GetHeight();
      wxSize size(width,height);
      //frame->GetClientSize(&width,&height);
      wxMDIChildFrame* frame = new wxMDIChildFrame(wxGetApp().getFrame(), wxID_ANY, "Preview", wxDefaultPosition, size);
      wxSize framesize = frame->ClientToWindowSize(size);
      frame->SetSize(framesize);
      ImageDisplay* display = new ImageDisplay(frame,image,size);
      //display->SetCursor(wxCursor(wxCURSOR_PENCIL));
      frame->Show(true);
      //puw->Activate();
               }
      break;
    default:
      wxMessageBox("Unhandled type clicked", "Implement me");
      break;
  }
}

void AdvTreeCtrl::OnItemMenu(wxTreeEvent& event){
  wxTreeItemId id = event.GetItem();
  AdvTreeItemData* item = (AdvTreeItemData*)GetItemData(id);
  wxPoint clientpt = event.GetPoint();
  wxMenu menu;
  menu.SetTitle("Blubb");
  //menu.Append(ID_AdvTreeCtrl, "&Dump");
  PopupMenu(&menu,clientpt);
}

BEGIN_EVENT_TABLE(AdvTreeCtrl, wxTreeCtrl)
EVT_TREE_ITEM_ACTIVATED(ID_AdvTreeCtrl, AdvTreeCtrl::OnItemActivated)
EVT_TREE_ITEM_MENU(ID_AdvTreeCtrl, AdvTreeCtrl::OnItemMenu)
END_EVENT_TABLE()

