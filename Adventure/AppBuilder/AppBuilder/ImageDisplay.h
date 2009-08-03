#ifndef IMAGE_DISPLAY_H
#define IMAGE_DISPLAY_H

#include <wx/scrolwin.h>
#include <wx/docmdi.h>
#include <wx/bitmap.h>

class ImageDisplay : public wxScrolledWindow{
public:
  ImageDisplay(wxWindow* parent, const wxImage& image, const wxSize& size);
  ~ImageDisplay();
  virtual void OnDraw(wxDC& dc);
  virtual void OnSize(wxSizeEvent& event);
protected:
  wxBitmap mBitmap;
  bool mShouldDraw;
  DECLARE_EVENT_TABLE();
};

class PopupWindow : public wxMDIChildFrame{
public:
  PopupWindow(wxMDIParentFrame* parent, wxWindowID id=-1, const wxPoint& pos=wxDefaultPosition, const wxSize& size = wxDefaultSize);
  virtual ~PopupWindow();
  virtual void OnDraw(wxDC& dc);
  virtual void OnActivate(wxActivateEvent& event);
  virtual void OnPaint(wxPaintEvent& event);
protected:
  ImageDisplay* mDisplay;
  DECLARE_EVENT_TABLE();
};

#endif
