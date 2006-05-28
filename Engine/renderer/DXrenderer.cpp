#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../window/nativeWindows.h"
#include "../system/engine.h"
#include "DXvertexbuffer.h"
#include "DXrenderer.h"
#include <d3dx9.h>

namespace Graphics{

DXRenderer::DXRenderer(): Renderer() {
  type_ = DirectX;
  d3d_ = NULL;
  device_ = NULL;
}

DXRenderer::~DXRenderer(){
}

void DXRenderer::initContext(::Windows::AppWindow* win){
  ::System::Log << "Initializing DirectX context\n";
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

  HWND wnd = dynamic_cast<::Windows::WindowsWindow*>(win)->getHandle();

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
      ::System::Log << "Hardware vertex-processing enabled\n";
  }
  else{
    ::System::Log << "Failed to enable pure DX9 device.\n";
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
  ::System::Log << "Initializing Scene\n";
  device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  device_->SetRenderState(D3DRS_LIGHTING, false);
  /*
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
  //device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  Renderer::initRendering();
}

void DXRenderer::renderScene(){
  // Clear the backbuffer and the zbuffer
  //device_->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
  //  D3DCOLOR_XRGB(128,128,128), 1.0f, 0 );

  // Begin the scene
  device_->BeginScene();

  Renderer::renderScene();

  // End the scene
  device_->EndScene();

  // Present the backbuffer contents to the display
  device_->Present( NULL, NULL, NULL, NULL );
}

void DXRenderer::resizeScene(int width, int height){
  if (device_ == NULL)
    return;
  ::System::Log << "Resizing Scene\n";
  win_->setWidth(width);
  win_->setHeight(height);
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

VertexBuffer* DXRenderer::createVertexBuffer(){
  return new DXVertexBuffer();
}

//! set lookAt
void DXRenderer::lookAt(const Vector3D* position, const Vector3D* look, const Vector3D* up){
  const D3DXVECTOR3 pos((float*)position);
  const D3DXVECTOR3 at((float*)look);
  const D3DXVECTOR3 u((float*)up);
  D3DXMATRIX V;
  D3DXMatrixLookAtRH(&V, &pos, &at, &u);
  device_->SetTransform(D3DTS_VIEW, &V);
}

//! set projection
void DXRenderer::projection(float angle, float aspect, float nearplane, float farplane){
  D3DXMATRIX proj;
  D3DXMatrixPerspectiveFovRH(&proj, angle/180.0f*D3DX_PI, aspect, nearplane, farplane);
  device_->SetTransform(D3DTS_PROJECTION, &proj);
}

//! set rendermode
void DXRenderer::renderMode(RendMode rm){
  if (rm == Filled){
    device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  }
  else if(rm == Wireframe){
    device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  }
}

}

#endif
