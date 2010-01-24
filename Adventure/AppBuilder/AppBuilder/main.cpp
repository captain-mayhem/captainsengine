#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/docmdi.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/cmdline.h>
#include <wx/fs_arc.h>
#include <wx/wfstream.h>

#include "main.h"
#include "AdvDoc.h"
#include "AdvMainTree.h"
#include "RenderWindow.h"
#include "Engine.h"
#include "Sound.h"

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

Application::Application() : mRunFile(""){
#ifdef WIN32
  AllocConsole();
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = 9000;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
  
  long stdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  int conHandle = _open_osfhandle(stdHandle, _O_TEXT);
  FILE* fp = _fdopen(conHandle, "w");
  *stdout = *fp;
  setvbuf(stdout, NULL, _IONBF, 0);

  stdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  conHandle = _open_osfhandle(stdHandle, _O_TEXT);
  fp = _fdopen(conHandle, "r");
  *stdin = *fp;
  setvbuf(stdin, NULL, _IONBF, 0);

  stdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  conHandle = _open_osfhandle(stdHandle, _O_TEXT);
  fp = _fdopen(conHandle, "w");
  *stderr = *fp;
  setvbuf(stderr, NULL, _IONBF, 0);

  std::ios::sync_with_stdio();
#endif
}

Application::~Application(){

}

bool Application::OnInit(){
  wxApp::OnInit();
  Engine::init();
  SoundEngine::init();
  mManager = new wxDocManager();
  wxSize sz(640,480);
  if (mRunFile == ""){
    new wxDocTemplate(mManager, "Adventure game", "game.dat", "", "dat", "Adventure runtime Doc", "Adventure View", CLASSINFO(AdvDocument), CLASSINFO(AdvMainTreeView));
    new wxDocTemplate(mManager, "Adventure project file", "*.adv", "", "adv", "Adventure Doc", "Adventure View", CLASSINFO(AdvDocument), CLASSINFO(AdvMainTreeView));
    mFrame = new MainFrame(mManager, NULL, "AppBuilder", wxPoint(0,0), wxSize(1024,768), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxMAXIMIZE);
  }
  else{
    new wxDocTemplate(mManager, "Adventure game", "game.dat", "", "dat", "Adventure runtime Doc", "Adventure View", CLASSINFO(AdvDocument), CLASSINFO(wxFrame));
    mFrame = new wxFrame(NULL, wxID_ANY, "Game", wxDefaultPosition, sz);
  }
  mFrame->Show(true);
  SetTopWindow(mFrame);
  wxInitAllImageHandlers();
  wxFileSystem::AddHandler(new wxArchiveFSHandler);
  if (mRunFile != ""){
    wxDocument* doc = mManager->CreateDocument(mRunFile, wxDOC_SILENT);
    if (doc == NULL){
      /*doc = new AdvDocument();
      wxFileInputStream strm(mRunFile);
      doc->LoadObject(strm);
      mManager->AddDocument(doc);
      if (doc == NULL){*/
        wxMessageBox("Game data not found. Program must exit.");
        wxExit();
        return false;
      //}
    }
    Engine::instance()->setData(static_cast<AdvDocument*>(doc));
    wxSize framesize = mFrame->ClientToWindowSize(sz);
    int attribs[] = {
      WX_GL_RGBA, //GL_TRUE,
      WX_GL_DOUBLEBUFFER,// GL_TRUE,
      //WX_GL_DEPTH_SIZE, 32,
      0
    };
    RenderWindow* rendwin = new RenderWindow(mFrame, attribs, sz.x, sz.y);
    rendwin->init();
    mFrame->SetSize(framesize);
    Engine::instance()->initGame();
  }
  return true;
}

int Application::OnExit(){
  delete mManager;
  SoundEngine::deinit();
  Engine::deinit();
  return 0;
}

void Application::OnInitCmdLine(wxCmdLineParser& parser){
  parser.AddOption("r", "run", "adventure file", wxCMD_LINE_VAL_STRING, 0);
  parser.AddSwitch("e", "edit", "adventure file", 0);
  wxApp::OnInitCmdLine(parser);
}

bool Application::OnCmdLineParsed(wxCmdLineParser& parser){
  wxString value;
  if (parser.Found("r", &value)){
    mRunFile = value;
  }
  else{
    mRunFile = "data/game.dat";
  }
  if (parser.Found("e")){
    mRunFile = "";
  }
  wxApp::OnCmdLineParsed(parser);
  return true;
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
  wxWindow* frame = new wxMDIChildFrame(wxGetApp().getFrame(), wxID_ANY, "Game", wxPoint(200,50), sz);
  wxSize framesize = frame->ClientToWindowSize(sz);
  //frame->SetSize(framesize);
  frame->Show(true);
  int attribs[] = {
    WX_GL_RGBA, //GL_TRUE,
    WX_GL_DOUBLEBUFFER,// GL_TRUE,
    //WX_GL_DEPTH_SIZE, 32,
    0
  };
  RenderWindow* rendwin = new RenderWindow(frame, attribs, sz.x, sz.y);
  rendwin->init();
  frame->SetSize(framesize);
  Engine::instance()->initGame();
}
