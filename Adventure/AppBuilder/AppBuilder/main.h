#include <wx/app.h>
#include <wx/docmdi.h>

class MainFrame : public wxDocMDIParentFrame{
public:
  MainFrame(wxDocManager* manager, wxFrame* frame, const wxString& title, const wxPoint& pos, const wxSize& size, long type);
  virtual ~MainFrame();
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnProjectSetup(wxCommandEvent& event);
private:
  DECLARE_EVENT_TABLE();
};

class Application : public wxApp{
public:
  Application();
  virtual ~Application();
  virtual bool OnInit();
  virtual int OnExit();
  wxDocMDIParentFrame* getFrame() {return mFrame;}
  wxDocument* getDocument() {return mManager->GetCurrentDocument();}
protected:
  wxDocManager* mManager;
  wxDocMDIParentFrame* mFrame;
};

DECLARE_APP(Application)
