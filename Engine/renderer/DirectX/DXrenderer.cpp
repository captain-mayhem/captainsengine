#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../../window/nativeWindows.h"
#include "../../system/engine.h"
#include "DXvertexbuffer.h"
#include "DXtexture.h"
#include "DXrenderer.h"
#include "DXindexbuffer.h"

using namespace CGE;
using namespace DirectX;

TR_CHANNEL(CGE_Renderer_DirectX)

DXRenderer::DXRenderer(): Renderer() {
  type_ = DirectX;
  mSwapchain = NULL;
  mBackBuffer = NULL;
  mD3d = NULL;
  mDevice = NULL;
  for (int i = 0; i < 3; ++i){
    mMatrix[i] = XMMatrixIdentity();
  }
  mMatrixMode = Modelview;
}

DXRenderer::~DXRenderer(){
}

void DXRenderer::initContext(AppWindow* win){
  TR_USE(CGE_Renderer_DirectX)
  TR_INFO("Initializing DirectX context");
  win_ = win;

  DXGI_SWAP_CHAIN_DESC scd;
  ZeroMemory(&scd, sizeof(scd));
  scd.BufferCount = 1;
  if (win->getBpp() == 16){
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  }
  else if (win->getBpp() == 32){
    scd.BufferDesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;
  }
  scd.BufferDesc.Width = win->getWidth();
  scd.BufferDesc.Height = win->getHeight();
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  HWND wnd = (HWND)win->getHandle();
  scd.OutputWindow = wnd;
  scd.SampleDesc.Count = 4;
  scd.Windowed = !win->isFullscreen();
  scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
    D3D11_SDK_VERSION, &scd, &mSwapchain, &mDevice, NULL, &mD3d))){
    TR_INFO("DirectX 11 context created successfully");
  }
  else{
    TR_ERROR("Failed to create DirectX 11 context");
  }

  if (win->isFullscreen())
    mSwapchain->SetFullscreenState(TRUE, NULL);

  ShowWindow(wnd, SW_SHOW);
  SetForegroundWindow(wnd);
  SetFocus(wnd);
  resizeScene(win->getWidth(), win->getHeight());
  //initRendering();  
}

void DXRenderer::killContext(){
  mSwapchain->SetFullscreenState(FALSE, NULL);
  SAFE_RELEASE(mSwapchain);
  SAFE_RELEASE(mBackBuffer);
  SAFE_RELEASE(mDevice);
  SAFE_RELEASE(mD3d);
}

void DXRenderer::initRendering(){
  TR_USE(CGE_Renderer_DirectX);
  TR_INFO("Initializing Scene");

  ID3D11Texture2D* backbuffer;
  mSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
  mDevice->CreateRenderTargetView(backbuffer, NULL, &mBackBuffer);
  backbuffer->Release();
  mD3d->OMSetRenderTargets(1, &mBackBuffer, NULL);
  //mD3d->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  //device_->SetRenderState(D3DRS_LIGHTING, false);

  //device_->SetRenderState(D3DRS_SPECULARENABLE, true);
  //device_->SetRenderState(D3DRS_NORMALIZENORMALS, true);
  //device_->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(51,51,51,255));
  /*
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
  //matrixtype_ = D3DTS_VIEW;

  Renderer::renderScene();

  // End the scene
  //device_->EndScene();

  // Present the backbuffer contents to the display
  //device_->Present( NULL, NULL, NULL, NULL );
}

void DXRenderer::resizeScene(int width, int height){
  TR_USE(CGE_Renderer_DirectX);
  if (mDevice == NULL)
    return;
  TR_INFO("Resizing Scene");

  D3D11_VIEWPORT viewport;
  ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = (FLOAT)width;
  viewport.Height = (FLOAT)height;
  mD3d->RSSetViewports(1, &viewport);
  
  win_->setWidth(width);
  win_->setHeight(height);
 
  Renderer::resizeScene(width, height);
}

//! clear scene
void DXRenderer::setClearColor(::CGE::Vector3D color){
  mClearColor[0] = color.x;
  mClearColor[1] = color.y;
  mClearColor[2] = color.z;
  mClearColor[3] = 1.0f;
}

void DXRenderer::clear(long flags){
  if (flags & COLORBUFFER)
    mD3d->ClearRenderTargetView(mBackBuffer, mClearColor);
  //if (flags & ZBUFFER)
    //mD3d->ClearDepthStencilView(, D3D11_CLEAR_DEPTH, 1.0f);
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
  XMVECTOR eye = XMVectorSet(position.x, position.y, position.z, 1.0f);
  XMVECTOR l = XMVectorSet(look.x, look.y, look.z, 1.0f);
  XMVECTOR u = XMVectorSet(up.x, up.y, up.z, 0.0f);
  XMMATRIX mat = XMMatrixLookAtRH(eye, l-eye, u);
  mMatrix[mMatrixMode] *= mat;
}

//! set projection
void DXRenderer::projection(float angle, float aspect, float nearplane, float farplane){
  mMatrix[Projection] = XMMatrixPerspectiveFovRH(angle/180.0f*XM_PI, aspect, nearplane, farplane);
}

void DXRenderer::ortho(const int width, const int height){
  mMatrix[Projection] = XMMatrixOrthographicRH((float)width, (float)height, 0, 1);
}

//! reset modelview matrix
void DXRenderer::resetModelView(){
  mMatrix[mMatrixMode] = XMMatrixIdentity();
}

//! translate
void DXRenderer::translate(float x, float y, float z){
  XMMATRIX mat = XMMatrixTranslation(x, y, z);
  mMatrix[mMatrixMode] *= mat;
}

//! scale
void DXRenderer::scale(float x, float y, float z){
  XMMATRIX mat = XMMatrixScaling(x, y, z);
  mMatrix[mMatrixMode] *= mat;
}

//! set rendermode
void DXRenderer::renderMode(RendMode rm){
  /*if (rm == Filled){
    device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  }
  else if(rm == Wireframe){
    device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
  }*/
}

//! set blending mode
void DXRenderer::blendFunc(BlendType src, BlendType dest){
  /*switch(src){
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
  }*/
}

//! enable blending
void DXRenderer::enableBlend(const bool flag){
  //device_->SetRenderState(D3DRS_ALPHABLENDENABLE, flag);
}

void DXRenderer::enableBackFaceCulling(const bool flag){
  /*D3DCULL cull = D3DCULL_NONE;
  if (flag)
    cull = D3DCULL_CW;
  device_->SetRenderState(D3DRS_CULLMODE, cull);*/
}

//! enable texturing
void DXRenderer::enableTexturing(const bool flag){
  //if (!flag)
  //  device_->SetTexture(0,0);
  //TODO
  //save texture when disabling to reenable it.
}

//! enable lighting
void DXRenderer::enableLighting(const bool flag){
  //device_->SetRenderState(D3DRS_LIGHTING, flag);
}
  
//! set color
void DXRenderer::setColor(float r, float g, float b, float a){
  //device_->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB((int)(a*255),(int)(r*255),(int)(g*255),(int)(b*255)));
}

void DXRenderer::setColor(const Color* c){
  //device_->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB((int)(c->a*255),(int)(c->r*255),(int)(c->g*255),(int)(c->b*255)));
}

//! push matrix
void DXRenderer::pushMatrix(){
  mMatrixStack[mMatrixMode].push(mMatrix[mMatrixMode]);
}

//! pop matrix
void DXRenderer::popMatrix(){
  mMatrix[mMatrixMode] = mMatrixStack[mMatrixMode].top();
  mMatrixStack[mMatrixMode].pop();
}

//! multiply matrix
void DXRenderer::multiplyMatrix(const CGE::Matrix& mat){
  XMMATRIX dxmat = XMMatrixSet(mat.getData()[0], mat.getData()[1], mat.getData()[2], mat.getData()[3],
    mat.getData()[4], mat.getData()[5], mat.getData()[6], mat.getData()[7],
    mat.getData()[8], mat.getData()[9], mat.getData()[10], mat.getData()[11],
    mat.getData()[12], mat.getData()[13], mat.getData()[14], mat.getData()[15]);
  mMatrix[mMatrixMode] *= dxmat;
}

//! set material
void DXRenderer::setMaterial(const Material& mat){
  /*D3DMATERIAL9 m;
  m.Diffuse = D3DXCOLOR(mat.getDiffuse().array);
  m.Ambient = D3DXCOLOR(mat.getAmbient().array);
  m.Specular = D3DXCOLOR(mat.getSpecular().array);
  m.Emissive = D3DXCOLOR(mat.getEmissive().array);
  m.Power = mat.getPower();
  device_->SetMaterial(&m);*/
}


//! get the viewport
void DXRenderer::getViewport(int view[4]){
  D3D11_VIEWPORT vp;
  UINT num = 1;
  mD3d->RSGetViewports(&num, &vp);
  view[0] = (int)vp.TopLeftX;
  view[1] = (int)vp.TopLeftY;
  view[2] = (int)(vp.TopLeftX + vp.Width);
  view[3] = (int)(vp.TopLeftY + vp.Height);
}

//! get a matrix
Matrix DXRenderer::getMatrix(MatrixType mt){
  XMMATRIX ret;
  if (mt == MVP){
    ret = mMatrix[Projection] * mMatrix[Modelview];
  }
  else
    ret = mMatrix[mt];
  float m[16];
  for (int i = 0; i < 4; ++i){
    XMVECTOR v = ret.r[i];
    memcpy(m+i*4*sizeof(float), v.m128_f32, 4 * sizeof(float));
  }
  return CGE::Matrix(m);
}

void DXRenderer::swapBuffers(){
    // Present the backbuffer contents to the display
    mSwapchain->Present(0, 0);
}

void DXRenderer::enableLight(short number, bool flag){
  //device_->LightEnable(number, flag);
}

void DXRenderer::setLight(int number, const Light& lit){
  /*D3DXCOLOR c(1.0,1.0,1.0,1.0);
  D3DXVECTOR3 dir(lit.getDirection().data);
  //D3DXVECTOR3 transdir;
  //D3DXMATRIX tmp, tmp2;
  //device_->GetTransform(D3DTS_VIEW, &tmp);
  //D3DXMatrixTranspose(&tmp2, &tmp);
  //D3DXMatrixInverse(&tmp2, NULL, &tmp);
  //D3DXVec3TransformNormal(&transdir, &dir, &tmp2);
  D3DLIGHT9 light;
  ZeroMemory(&light, sizeof(light));
  light.Type = D3DLIGHT_DIRECTIONAL;
  light.Ambient = D3DXCOLOR(0.0,0.0,0.0,1.0);
  //light.Diffuse = D3DXCOLOR(0.1,0.0,0.0,1.0);
  light.Diffuse = D3DXCOLOR(1.0,1.0,1.0,1.0);
  light.Specular = D3DXCOLOR(1.0,1.0,1.0,1.0);
  //light.Specular = c;
  light.Direction = dir;
  device_->SetLight(number, &light);*/
}

void DXRenderer::switchFromViewToModelTransform(){
  //matrixtype_ = D3DTS_WORLD;
}

