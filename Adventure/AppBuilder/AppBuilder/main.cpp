#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/docmdi.h>
#include <wx/notebook.h>
#include <wx/statline.h>

#include "main.h"
#include "AdvDoc.h"
#include "AdvMainTree.h"
#include "RenderWindow.h"
#include "Engine.h"

enum{
  ID_Quit=1,
  ID_About,
  ID_Project_Setup,
  ID_Create_Game,
};

BEGIN_EVENT_TABLE(MainFrame, wxDocMDIParentFrame)
EVT_MENU(ID_Quit, MainFrame::OnQuit)
EVT_MENU(ID_About, MainFrame::OnAbout)
EVT_MENU(ID_Project_Setup, MainFrame::OnProjectSetup)
EVT_MENU(ID_Create_Game, MainFrame::OnCreateGame)
END_EVENT_TABLE()

IMPLEMENT_APP(Application)

Application::Application(){
}

Application::~Application(){

}

bool Application::OnInit(){
  Engine::init();
  mManager = new wxDocManager();
  new wxDocTemplate(mManager, "Adventure", "*.adv", "", "adv", "Adventure Doc", "Adventure View", CLASSINFO(AdvDocument), CLASSINFO(AdvMainTreeView));
  mFrame = new MainFrame(mManager, NULL, "AppBuilder", wxPoint(50,50), wxSize(800,600), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);
  mFrame->Show(true);
  SetTopWindow(mFrame);
  wxInitAllImageHandlers();
  return true;
}

int Application::OnExit(){
  delete mManager;
  Engine::deinit();
  return 0;
}

MainFrame::MainFrame(wxDocManager* manager, wxFrame* frame, const wxString& title, const wxPoint& pos, const wxSize& size, long type) :
wxDocMDIParentFrame(manager, frame, wxID_ANY, title, pos, size, type, "myFrame"){
  wxMenu* menuFile = new wxMenu();
  menuFile->Append(wxID_OPEN, "&Open...");
  menuFile->AppendSeparator();
  menuFile->Append(ID_Create_Game, "&Create Game...");
  manager->FileHistoryUseMenu(menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT, "&Exit");

  wxMenu* menuSetup = new wxMenu();
  menuSetup->Append(ID_Project_Setup, "&Project Setup");

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(ID_About, "&About");

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, "&Project");
  menuBar->Append(menuSetup, "&Setup");
  menuBar->Append(menuHelp, "&Help");
  SetMenuBar(menuBar);

  CreateStatusBar();
  SetStatusText("AppBuilder started!");
}

MainFrame::~MainFrame(){

}

void MainFrame::OnQuit(wxCommandEvent& event){
  Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event){
  wxMessageBox("AppBuilder v0.1", "About AppBuilder", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnProjectSetup(wxCommandEvent& event){
  wxDialog dialog(this, wxID_ANY, "Project Setup", wxDefaultPosition, wxSize(500,500));
  wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
  wxNotebook* tabs = new wxNotebook(&dialog, wxID_ANY, wxPoint(0,0), wxSize(300,450), wxNB_TOP);
  wxPanel* general = new wxPanel(tabs);
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  wxStaticText* txt = new wxStaticText(general, wxID_ANY, "Projectname:");
  wxTextCtrl* prname = new wxTextCtrl(general, wxID_ANY, "Project");
  wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL,5);
  sizer->Add(txt, flags);
  sizer->Add(prname, flags);
  txt = new wxStaticText(general, wxID_ANY, "Game resolution");
  sizer->Add(txt, flags);
  wxString res[5];
  res[0] = "320x200";
  res[1] = "320x240";
  res[2] = "640x480";
  res[3] = "800x600";
  res[4] = "1024x768";
  wxChoice* resolution = new wxChoice(general, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, res);
  resolution->SetSelection(2);
  sizer->Add(resolution, flags);
  wxCheckBox* aliasing = new wxCheckBox(general, wxID_ANY, "No antialiasing");
  sizer->Add(aliasing, flags);
  wxCheckBox* mutemusic = new wxCheckBox(general, wxID_ANY, "Mute music when speech");
  mutemusic->SetValue(true);
  sizer->Add(mutemusic, flags);
  wxBoxSizer* horisizer = new wxBoxSizer(wxHORIZONTAL);
  horisizer->Add(sizer);
  wxStaticLine* middleline = new wxStaticLine(general, wxID_ANY, wxDefaultPosition, wxSize(3,450), wxLI_VERTICAL);
  horisizer->Add(middleline,wxSizerFlags().Center());
  wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
  wxCheckBox* texton = new wxCheckBox(general, wxID_ANY, "Text on/off");
  sizer2->Add(texton, flags);
  wxCheckBox* draggedicons = new wxCheckBox(general, wxID_ANY, "Draw dragged Item-icons");
  sizer2->Add(draggedicons, flags);
  horisizer->Add(sizer2);
  general->SetSizer(horisizer);
  
  wxPanel* script = new wxPanel(tabs);
  wxPanel* menu = new wxPanel(tabs);
  tabs->AddPage(general, "General", true);
  tabs->AddPage(script, "Script and taskbar");
  tabs->AddPage(menu, "Menu and colors");
  topsizer->Add(tabs, wxSizerFlags().Expand());
  wxButton* ok = new wxButton(&dialog, wxID_OK, "OK");
  topsizer->Add(ok, wxSizerFlags().Center());
  dialog.SetSizer(topsizer);
  dialog.ShowModal();
}

void MainFrame::OnCreateGame(wxCommandEvent& event){
  wxSize sz(640,480);
  wxMDIChildFrame* frame = new wxMDIChildFrame(wxGetApp().getFrame(), wxID_ANY, "Game", wxDefaultPosition, sz);
  wxSize framesize = frame->ClientToWindowSize(sz);
  frame->SetSize(framesize);
  frame->Show(true);
  int attribs[] = {
    WX_GL_RGBA, //GL_TRUE,
    WX_GL_DOUBLEBUFFER,// GL_TRUE,
    //WX_GL_DEPTH_SIZE, 32,
    NULL
  };
  RenderWindow* rendwin = new RenderWindow(frame, attribs, sz.x, sz.y);
  rendwin->init();
  Engine::instance()->initGame();
  Engine::instance()->loadRoom("FirstRoom");
}
