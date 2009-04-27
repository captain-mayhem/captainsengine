#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../../window/nativeWindows.h"
#include "../../system/engine.h"
#include "DXvertexbuffer.h"
#include "DXtexture.h"
#include "DXrenderer.h"
#include <d3dx9.h>
#include "DXindexbuffer.h"

using namespace CGE;

DXRenderer::DXRenderer(): Renderer() {
  type_ = DirectX;
  d3d_ = NULL;
  device_ = NULL;
}

DXRenderer::~DXRenderer(){
}

void DXRenderer::initContext(::Windows::AppWindow* win){
  CGE::Log << "Initializing DirectX context\n";
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
  ppm.AutoDepthStencilFormat = D3DFMT_D24X8;
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
    D3DCREATE_HARDWARE_VERTEXPROCESSING /*| D3DCREATE_PUREDEVICE*/,
    &ppm, &device_))){
      CGE::Log << "Hardware vertex-processing enabled\n";
  }
  else{
    CGE::Log << "Failed to enable pure DX9 device.\n";
  }
  

  ShowWindow(wnd, SW_SHOW);
  SetForegroundWindow(wnd);
  SetFocus(wnd);
  resizeScene(win->getWidth(), win->getHeight());
  //initRendering();  
}

void DXRenderer::killContext(){
  SAFE_RELEASE(device_);
  SAFE_RELEASE(d3d_);
}

void DXRenderer::initRendering(){
  CGE::Log << "Initializing Scene\n";
  device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  device_->SetRenderState(D3DRS_LIGHTING, false);

  device_->SetRenderState(D3DRS_SPECULARENABLE, true);
  device_->SetRenderState(D3DRS_NORMALIZENORMALS, true);
  /*
  //smooth shading
  glShadeModel(GL_SMOOTH);

  //depth buffer
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  //better perspective calculations
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);*/
  //device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  device_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  device_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  device_->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
  //device_->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(0,255,0,255));
  // Coloring
  device_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
  device_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
  device_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  device_->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
  device_->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
  device_->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
  device_->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);

  // Alpha transparency
  device_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
  device_->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  device_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  device_->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
  device_->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
  device_->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  device_->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  /*device_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
  device_->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  device_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  device_->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);*/
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
  //device_->EndScene();

  // Present the backbuffer contents to the display
  //device_->Present( NULL, NULL, NULL, NULL );
}

void DXRenderer::resizeScene(int width, int height){
  if (device_ == NULL)
    return;
  CGE::Log << "Resizing Scene\n";
  if (height == 0){
    height = 1;
  }
  
  win_->setWidth(width);
  win_->setHeight(height);
 
  Renderer::resizeScene(width, height);
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
void DXRenderer::setClearColor(::CGE::Vector3D color){
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

IndexBuffer* DXRenderer::createIndexBuffer(IndexBuffer::Type t, uint32 size){
  return new DXIndexBuffer(t,size);
}

Texture* DXRenderer::createTexture(string filename){
  return new DXTexture(filename);
}


//! set lookAt
void DXRenderer::lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up){
  const D3DXVECTOR3 pos(&position.x);
  const D3DXVECTOR3 at(&look.x);
  const D3DXVECTOR3 u(&up.x);
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

void DXRenderer::ortho(const int width, const int height){
  D3DXMATRIX orth;
  D3DXMatrixOrthoRH(&orth, static_cast<float>(width), static_cast<float>(height), 0, 1);
  device_->SetTransform(D3DTS_PROJECTION, &orth);
}

//! reset modelview matrix
void DXRenderer::resetModelView(){
  D3DXMATRIX id;
  D3DXMatrixIdentity(&id);
  device_->SetTransform(D3DTS_VIEW, &id);
  device_->SetTransform(D3DTS_WORLD, &id);
}

//! translate
void DXRenderer::translate(float x, float y, float z){
  D3DXMATRIX trans;
  D3DXMatrixTranslation(&trans, x, y, z);
  device_->MultiplyTransform(D3DTS_WORLD, &trans);
}

//! scale
void DXRenderer::scale(float x, float y, float z){
  D3DXMATRIX scal;
  D3DXMatrixScaling(&scal, x, y, z);
  device_->MultiplyTransform(D3DTS_WORLD, &scal);
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

//! set blending mode
void DXRenderer::blendFunc(BlendType src, BlendType dest){
  switch(src){
    case BLEND_ONE:
      device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
      break;
    case BLEND_SRC_ALPHA:
      device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
      break;
  }
  switch(dest){
    case BLEND_ONE:
      device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
      break;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      break; 
  }
}

//! enable blending
void DXRenderer::enableBlend(const bool flag){
  device_->SetRenderState(D3DRS_ALPHABLENDENABLE, flag);
}

void DXRenderer::enableBackFaceCulling(const bool flag){
  D3DCULL cull = D3DCULL_NONE;
  if (flag)
    cull = D3DCULL_CW;
  device_->SetRenderState(D3DRS_CULLMODE, cull);
}

//! enable texturing
void DXRenderer::enableTexturing(const bool flag){
  if (!flag)
    device_->SetTexture(0,0);
  //TODO
  //save texture when disabling to reenable it.
}

//! enable lighting
void DXRenderer::enableLighting(const bool flag){
  device_->SetRenderState(D3DRS_LIGHTING, flag);
}
  
//! set color
void DXRenderer::setColor(float r, float g, float b, float a){
  device_->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB((int)(a*255),(int)(r*255),(int)(g*255),(int)(b*255)));
}

void DXRenderer::setColor(const Color* c){
  device_->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB((int)(c->a*255),(int)(c->r*255),(int)(c->g*255),(int)(c->b*255)));
}

//! push matrix
void DXRenderer::pushMatrix(){
  D3DXMATRIX mat;
  device_->GetTransform(D3DTS_WORLD, &mat);
  modelstack_.push(mat);
  device_->GetTransform(D3DTS_VIEW, &mat);
  modelstack_.push(mat);
}

//! pop matrix
void DXRenderer::popMatrix(){
  D3DXMATRIX mat = modelstack_.top();
  device_->SetTransform(D3DTS_VIEW, &mat);
  modelstack_.pop();
  mat = modelstack_.top();
  device_->SetTransform(D3DTS_WORLD, &mat);
  modelstack_.pop();
}

//! multiply matrix
void DXRenderer::multiplyMatrix(const CGE::Matrix& mat){
  D3DXMATRIX matrix(mat.getData());
  device_->MultiplyTransform(D3DTS_WORLD, &matrix);
}

//! set material
void DXRenderer::setMaterial(const Material& mat){
  D3DMATERIAL9 m;
  m.Diffuse.r = mat.getDiffuse().r*255; m.Diffuse.g = mat.getDiffuse().g*255; m.Diffuse.b = mat.getDiffuse().b*255; m.Diffuse.a = mat.getDiffuse().a*255;
  m.Ambient.r = mat.getAmbient().r*255; m.Ambient.g = mat.getAmbient().g*255; m.Ambient.b = mat.getAmbient().b*255; m.Ambient.a = mat.getAmbient().a*255;
  m.Specular.r = mat.getSpecular().r*255; m.Specular.g = mat.getSpecular().g*255; m.Specular.b = mat.getSpecular().b*255; m.Specular.a = mat.getSpecular().a*255;
  m.Emissive.r = mat.getEmissive().r*255; m.Emissive.g = mat.getEmissive().g*255; m.Emissive.b = mat.getEmissive().b*255; m.Emissive.a = mat.getEmissive().a*255;
  m.Power = mat.getPower();
  device_->SetMaterial(&m);
}


//! get the viewport
void DXRenderer::getViewport(int view[4]){
  D3DVIEWPORT9 vp;
  device_->GetViewport(&vp);
  view[0] = vp.X;
  view[1] = vp.Y;
  view[2] = vp.X+vp.Width;
  view[3] = vp.Y+vp.Height;
}

//! get a matrix
Matrix DXRenderer::getMatrix(MatrixType mt){
  if (mt == Projection){
    D3DXMATRIX proj;
    device_->GetTransform(D3DTS_PROJECTION, &proj);
    return Matrix((float*)proj.m).transpose();
  }
  else if (mt == Modelview){
    D3DXMATRIX tmp;
    device_->GetTransform(D3DTS_VIEW, &tmp);
    Matrix view = Matrix(((float*)tmp.m))/*.transpose()*/;
    device_->GetTransform(D3DTS_WORLD, &tmp);
    Matrix wrld = Matrix(((float*)tmp.m))/*.transpose()*/;
    return wrld*view;
  }
  return Matrix(Matrix::Identity);
}

void DXRenderer::swapBuffers(){
    // End the scene
    device_->EndScene();

    // Present the backbuffer contents to the display
    device_->Present( NULL, NULL, NULL, NULL );
}

void DXRenderer::enableLight(short number, bool flag){
  device_->LightEnable(number, flag);
}

void DXRenderer::setLight(int number, const Light& lit){
  D3DXCOLOR c(1.0,1.0,1.0,1.0);
  D3DXVECTOR3 dir(lit.getDirection().data);
  D3DXVECTOR3 transdir;
  D3DXMATRIX tmp, tmp2;
  device_->GetTransform(D3DTS_WORLD, &tmp);
  //D3DXMatrixTranspose(&tmp2, &tmp);
  D3DXMatrixInverse(&tmp2, NULL, &tmp);
  D3DXVec3TransformNormal(&transdir, &dir, &tmp2);
  D3DLIGHT9 light;
  ZeroMemory(&light, sizeof(light));
  light.Type = D3DLIGHT_DIRECTIONAL;
  light.Diffuse = c;
  light.Direction = transdir;
  device_->SetLight(number, &light);
}

