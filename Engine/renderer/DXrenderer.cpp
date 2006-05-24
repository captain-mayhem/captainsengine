#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../window/nativeWindows.h"
#include "../system/engine.h"
#include "DXrenderer.h"

namespace Renderer{

DXRenderer::DXRenderer(): Renderer() {
  type_ = DirectX;
  d3d_ = NULL;
  device_ = NULL;
}

DXRenderer::~DXRenderer(){
}

void DXRenderer::initContext(::Window::Window* win){
  ::Engine::Log << "Initializing DirectX context\n";
  win_ = win;

  d3d_ = Direct3DCreate9(D3D_SDK_VERSION);

  D3DDISPLAYMODE dm;
  D3DFORMAT format;

  if (!win->isFullscreen())
    d3d_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);

  if (win->getBpp() == 16){
    if(SUCCEEDED(d3d_->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      D3DFMT_R5G6B5, D3DFMT_R5G6B5, FALSE)))
        format = D3DFMT_R5G6B5;
    else if(SUCCEEDED(d3d_->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      D3DFMT_X1R5G5B5, D3DFMT_X1R5G5B5, FALSE)))
      format = D3DFMT_X1R5G5B5;
    else if(SUCCEEDED(d3d_->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, FALSE)))
      format = D3DFMT_A1R5G5B5;
  }
  else if (win->getBpp() == 32){
    if(SUCCEEDED(d3d_->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, FALSE)))
      format = D3DFMT_X8R8G8B8;
    else if(SUCCEEDED(d3d_->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
      D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, FALSE)))
      format = D3DFMT_A8R8G8B8;
  }

  D3DPRESENT_PARAMETERS ppm;
  ZeroMemory(&ppm, sizeof(ppm));

  ppm.Windowed = !win->isFullscreen();
  ppm.BackBufferCount = 1;
  ppm.AutoDepthStencilFormat = D3DFMT_D16;
  ppm.EnableAutoDepthStencil = TRUE;

  HWND wnd = dynamic_cast<::Window::WindowsWindow*>(win)->getHandle();

  if(win->isFullscreen()){
    ppm.SwapEffect = D3DSWAPEFFECT_FLIP;
    ppm.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    ppm.hDeviceWindow = wnd;
    ppm.BackBufferWidth = win->getWidth();
    ppm.BackBufferHeight = win->getHeight();
    ppm.BackBufferFormat = format;
  }
  else{
    ppm.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    ppm.SwapEffect = D3DSWAPEFFECT_DISCARD;
    ppm.BackBufferFormat = dm.Format;
  }

  if(SUCCEEDED(d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd,
    D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE,
    &ppm, &device_))){
      ::Engine::Log << "Hardware vertex-processing enabled.\n";
  }
  else{
    ::Engine::Log << "Failed to enable pure DX9 device.\n";
  }
  

  ShowWindow(wnd, SW_SHOW);
  SetForegroundWindow(wnd);
  SetFocus(wnd);
  resizeScene(win->getWidth(), win->getHeight());
  initRendering();  
  
}

void DXRenderer::killContext(){
  SAFE_RELEASE(device_);
  SAFE_RELEASE(d3d_);
}

void DXRenderer::initRendering(){
  ::Engine::Log << "Initializing Scene\n";/*
  //smooth shading
  glShadeModel(GL_SMOOTH);

  //background color black
  glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

  //depth buffer
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  //better perspective calculations
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);*/
  Renderer::initRendering();
}

void DXRenderer::renderScene(){
  // Clear the backbuffer and the zbuffer
  //device_->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
  //  D3DCOLOR_XRGB(128,128,128), 1.0f, 0 );

  // Begin the scene
  device_->BeginScene();

  Renderer::renderScene();

  //Font1->DrawTextScaled(-0.55f, 0.0f, 0.9f, 0.06f, 0.02f, D3DCOLOR_XRGB(0,0,250),          
  //  "Hello to all Game Programmers", D3DFONT_FILTERED);  

  //sprintf( strBuffer, "Framerate:  %03d", (long)FrameRate );                                 
  //Font1->DrawTextScaled(-0.8f, -0.8f, 0.9f, 0.03f, 0.02f, D3DCOLOR_XRGB(250,0,0), strBuffer, D3DFONT_FILTERED ); 

  //sprintf( strBuffer, "Resolution:  %d, %d, %d", screenwidth, screenheight, Farbtiefe);                                 
  //Font1->DrawTextScaled(-0.8f, -0.7f, 0.9f, 0.03f, 0.02f, D3DCOLOR_XRGB(250,0,0), strBuffer, D3DFONT_FILTERED ); 

  //sprintf( strBuffer, "Blickweite:  %0.1f", SizeOfUniverse );                                 
  //Font1->DrawTextScaled(-0.8f, -0.6f, 0.9f, 0.03f, 0.02f, D3DCOLOR_XRGB(250,0,0), strBuffer, D3DFONT_FILTERED ); 

  // End the scene
  device_->EndScene();

  // Present the backbuffer contents to the display
  device_->Present( NULL, NULL, NULL, NULL );
}

void DXRenderer::resizeScene(int width, int height){
  if (device_ == NULL)
    return;
  ::Engine::Log << "Resizing Scene\n";
  win_->setWidth(width);
  ein_->setHeight(height);
  /*
  if (height == 0){
    height = 1;
  }
  glViewport(0, 0, width, height);

  //Projection Matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //Calculate Aspect Ratio
  gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,100.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();*/
}

//! clear scene
void DXRenderer::setClearColor(::Math::Vector3D color){
  clearColor_ = color*255;
}

void DXRenderer::clear(long flags){
  long dxflags = 0;
  if (flags & ZBUFFER)
    dxflags |= D3DCLEAR_ZBUFFER;
  if (flags & COLORBUFFER)
    dxflags |= D3DCLEAR_TARGET;
  device_->Clear(0, NULL, dxflags, D3DCOLOR_XRGB((int)clearColor_.x, (int)clearColor_.y, (int)clearColor_.z)
    , 1.0f, 0);
}

}
#endif
