
#include <wx/glcanvas.h>
#include <wx/timer.h>

class RenderWindow : public wxGLCanvas{
public:
  RenderWindow(wxWindow* parent, int* attriblist, int resx, int resy);
  virtual ~RenderWindow();
  void OnSize(wxSizeEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnEnterWindow(wxMouseEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnTimer(wxTimerEvent& event);
  void OnIdle(wxIdleEvent& event);
  bool init();
protected:
  wxGLContext* mContext;
  wxTimer mTimer;
  DECLARE_EVENT_TABLE();
};