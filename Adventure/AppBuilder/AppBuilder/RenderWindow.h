
#include <wx/glcanvas.h>
#include <wx/timer.h>

class RenderWindow : public wxGLCanvas{
public:
  RenderWindow(wxWindow* parent, int* attriblist, int resx, int resy);
  virtual ~RenderWindow();
  void OnSize(wxSizeEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnEnterWindow(wxMouseEvent& event);
  void OnExitWindow(wxMouseEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseLeft(wxMouseEvent& event);
  void OnMouseRight(wxMouseEvent& event);
  void OnTimer(wxTimerEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  bool init();
protected:
  wxGLContext* mContext;
  wxTimer mTimer;
  bool mRendering;
  DECLARE_EVENT_TABLE();
};

