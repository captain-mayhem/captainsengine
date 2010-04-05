#include "ImageDisplay.h"
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/mdi.h>

BEGIN_EVENT_TABLE(PopupWindow, wxMDIChildFrame)
EVT_PAINT(PopupWindow::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ImageDisplay, wxScrolledWindow)
//EVT_PAINT(ImageDisplay::OnPaint)
EVT_SIZE(ImageDisplay::OnSize)
END_EVENT_TABLE()

ImageDisplay::ImageDisplay(wxWindow* parent, const wxImage& image, const wxSize& size):
wxScrolledWindow(parent,wxID_ANY,wxPoint(0,0),size){
  SetBackgroundColour(wxColour(_T("BLACK")));
  mBitmap = wxBitmap(image);
  mShouldDraw = false;
}

ImageDisplay::~ImageDisplay(){

}

void ImageDisplay::OnDraw(wxDC& dc){
  wxCoord w,h;
  dc.GetSize(&w,&h);
  double scalex = w/(double)mBitmap.GetWidth();
  double scaley = h/(double)mBitmap.GetHeight();
  double scale = std::min(scalex,scaley);
  dc.SetUserScale(scale,scale);
  dc.DrawBitmap(mBitmap,0,0,true);
  dc.SetTextForeground(wxColour(_T("WHITE")));
  dc.DrawText(wxT("Test"),wxPoint(10,10));
}

void ImageDisplay::OnSize(wxSizeEvent& event){
  Refresh();
}

PopupWindow::PopupWindow(wxMDIParentFrame* parent, wxWindowID id/* =-1 */, const wxPoint& pos/* =wxDefaultPosition */, const wxSize& size /* = wxDefaultSize */) :
wxMDIChildFrame(parent,id,wxT("Preview"),pos,size){
  int width, height;
  GetClientSize(&width,&height);
  //mDisplay = new ImageDisplay(this, wxPoint(0,0), wxSize(width,height));
  //SetBackgroundColour(wxColour(_T("BLACK")));
  mDisplay->Show(true);
}

PopupWindow::~PopupWindow(){
  //delete mDisplay;
}

void PopupWindow::OnActivate(wxActivateEvent& event){
  if (event.GetActive() && mDisplay)
    mDisplay->SetFocus();
}

void PopupWindow::OnDraw(wxDC& dc){
  //mDisplay->OnDraw(dc);
  //dc.DrawText("Test",wxPoint(10,10));
}

void PopupWindow::OnPaint(wxPaintEvent& event){
  //wxPaintDC dc(this);
  //mDisplay->OnDraw(dc);
  //dc.DrawText("Test",wxPoint(10,10));
}

