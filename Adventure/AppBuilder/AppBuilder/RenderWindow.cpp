#include "RenderWindow.h"
#include <wx/dcclient.h>
#include "Engine.h"

BEGIN_EVENT_TABLE(RenderWindow, wxGLCanvas)
EVT_SIZE(RenderWindow::OnSize)
EVT_PAINT(RenderWindow::OnPaint)
END_EVENT_TABLE()

RenderWindow::RenderWindow(wxWindow* parent, int* attriblist, int resx, int resy) : 
wxGLCanvas(parent, wxID_ANY, attriblist, wxPoint(100,100), wxSize(resx, resy), wxFULL_REPAINT_ON_RESIZE),
mContext(NULL){
  
}

RenderWindow::~RenderWindow(){
  delete mContext;
}

bool RenderWindow::init(){
  mContext = new wxGLContext(this, NULL);
  SetCurrent(*mContext);
  glMatrixMode(GL_PROJECTION);
  glOrtho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  glMatrixMode(GL_MODELVIEW);
  glClearColor(1.0,0.0,0.0,1.0);
  glColor4ub(255,255,255,255);

  glDisable(GL_DEPTH_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glAlphaFunc(GL_GREATER, 0);
  glEnable(GL_ALPHA_TEST);
  return true;
}

void RenderWindow::OnSize(wxSizeEvent& event){
  wxGLCanvas::OnSize(event);
  int w, h;
  GetClientSize(&w, &h);
  SetCurrent(*mContext);
  glViewport(0, 0, w, h);
}

void RenderWindow::OnPaint(wxPaintEvent& event){
  wxPaintDC dc(this);
  SetCurrent(*mContext);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  Engine::instance()->render();

  glFlush();
  SwapBuffers();
}

void RenderWindow::OnEnterWindow(wxMouseEvent& event){
  SetFocus();
}

void RenderWindow::OnEraseBackground(wxEraseEvent& event){

}
