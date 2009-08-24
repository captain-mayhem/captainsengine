#include "RenderWindow.h"
#include <wx/dcclient.h>
#include "Engine.h"

BEGIN_EVENT_TABLE(RenderWindow, wxGLCanvas)
EVT_SIZE(RenderWindow::OnSize)
EVT_PAINT(RenderWindow::OnPaint)
//EVT_TIMER(wxID_ANY, RenderWindow::OnTimer)
EVT_MOTION(RenderWindow::OnMouseMove)
EVT_LEFT_UP(RenderWindow::OnMouseLeft)
EVT_RIGHT_UP(RenderWindow::OnMouseRight)
EVT_ENTER_WINDOW(RenderWindow::OnEnterWindow)
EVT_LEAVE_WINDOW(RenderWindow::OnExitWindow)
EVT_CLOSE(RenderWindow::OnClose)
END_EVENT_TABLE()

RenderWindow::RenderWindow(wxWindow* parent, int* attriblist, int resx, int resy) : 
wxGLCanvas(parent, wxID_ANY, attriblist, wxPoint(100,100), wxSize(resx, resy), wxFULL_REPAINT_ON_RESIZE),
mContext(NULL), mRendering(false){
  
}

RenderWindow::~RenderWindow(){
  if (!mRendering)
    return;
  Disconnect(wxEVT_IDLE, wxIdleEventHandler(RenderWindow::OnIdle));
  mRendering = false;
  Engine::instance()->exitGame();
  delete mContext;
}

bool RenderWindow::init(){
  //mTimer.SetOwner(this);
  //mTimer.Start(50, false);
  mContext = new wxGLContext(this, NULL);
  SetCurrent(*mContext);
  glMatrixMode(GL_PROJECTION);
  glOrtho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0,0.0,0.0,1.0);
  glColor4ub(255,255,255,255);

  glDisable(GL_DEPTH_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glAlphaFunc(GL_GREATER, 0);
  glEnable(GL_ALPHA_TEST);

  mRendering = true;
  Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(RenderWindow::OnIdle));
  return true;
}

void RenderWindow::OnSize(wxSizeEvent& event){
  wxGLCanvas::OnSize(event);
  int w, h;
  GetClientSize(&w, &h);
  if (!mRendering)
    return;
  SetCurrent(*mContext);
  glViewport(0, 0, w, h);
}

void RenderWindow::OnPaint(wxPaintEvent& event){
  wxPaintDC dc(this);

  if (!GetParent()->IsShown())
    return;

  SetCurrent(*mContext);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  Engine::instance()->render();

  SwapBuffers();
  //Refresh(false);
}

void RenderWindow::OnEnterWindow(wxMouseEvent& event){
  SetFocus();
  SetCursor(wxCursor(wxCURSOR_BLANK));
}

void RenderWindow::OnExitWindow(wxMouseEvent& event){
  SetCursor(wxCursor(wxCURSOR_ARROW));
}

void RenderWindow::OnEraseBackground(wxEraseEvent& event){

}

void RenderWindow::OnMouseMove(wxMouseEvent& event){
  Vec2i pos(event.GetX(), event.GetY());
  Engine::instance()->setCursorPos(pos);
}

void RenderWindow::OnMouseLeft(wxMouseEvent& event){
  Vec2i pos(event.GetX(), event.GetY());
  Engine::instance()->leftClick(pos);
}

void RenderWindow::OnMouseRight(wxMouseEvent& event){
  Vec2i pos(event.GetX(), event.GetY());
  //Engine::instance()->leftClick(pos);
}

void RenderWindow::OnTimer(wxTimerEvent& event){
  Refresh(false);
}

void RenderWindow::OnClose(wxCloseEvent& event){
  Disconnect(wxEVT_IDLE, wxIdleEventHandler(RenderWindow::OnIdle));
  mRendering = false;
  delete mContext;
  event.Skip();
}

void RenderWindow::OnIdle(wxIdleEvent& event){

  if (!GetParent()->IsShown())
    return;

  SetCurrent(*mContext);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  Engine::instance()->render();

  SwapBuffers();

  event.RequestMore();
}
