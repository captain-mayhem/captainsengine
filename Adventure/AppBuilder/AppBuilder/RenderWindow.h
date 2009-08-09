
#include <wx/glcanvas.h>

class RenderWindow : public wxGLCanvas{
public:
  RenderWindow(wxWindow* parent, int* attriblist, int resx, int resy);
  virtual ~RenderWindow();
  void OnSize(wxSizeEvent& event);
  void OnPaint(wxPaintEvent& event);
  void OnEnterWindow(wxMouseEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  bool init();
protected:
  wxGLContext* mContext;
  DECLARE_EVENT_TABLE();
};