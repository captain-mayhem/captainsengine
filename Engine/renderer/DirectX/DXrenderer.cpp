#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../../window/nativeWindows.h"
#include "../../system/engine.h"
#include "DXvertexbuffer.h"
#include "DXtexture.h"
#include "DXrenderer.h"
#include "DXindexbuffer.h"
#include "DXshader.h"

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
    XMMATRIX mat = XMMatrixIdentity();
    XMStoreFloat4x4(&mMatrix[i], mat);
  }
  mMatrixMode = Modelview;
}

DXRenderer::~DXRenderer(){
}

void DXRenderer::initContext(AppWindow* win){
  TR_USE(CGE_Renderer_DirectX)
  TR_INFO("Initializing DirectX context");
  win_ = win;
  HWND wnd = (HWND)win->getHandle();
  ShowWindow(wnd, SW_SHOW);
  SetForegroundWindow(wnd);
  SetFocus(wnd);

  DXGI_SWAP_CHAIN_DESC scd;
  ZeroMemory(&scd, sizeof(scd));
  scd.BufferCount = 1;
  if (win->getBpp() == 32){
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  }
  else if (win->getBpp() == 16){
    scd.BufferDesc.Format = DXGI_FORMAT_B4G4R4A4_UNORM;
  }
  scd.BufferDesc.Width = win->getWidth();
  scd.BufferDesc.Height = win->getHeight();
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  
  scd.OutputWindow = wnd;
  scd.SampleDesc.Count = 4;
  //scd.SampleDesc.Quality = 0;
  scd.Windowed = !win->isFullscreen();
  scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  D3D_FEATURE_LEVEL level;

  /*HRESULT res = D3D11CreateDevice(NULL,
    D3D_DRIVER_TYPE_HARDWARE,
    NULL,
    D3D11_CREATE_DEVICE_DEBUG,
    NULL,
    0,
    D3D11_SDK_VERSION,
    &mDevice,
    &level,
    &mD3d);*/

  HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0,
    D3D11_SDK_VERSION, &scd, &mSwapchain, &mDevice, &level, &mD3d);
  if (SUCCEEDED(res)){
    TR_INFO("DirectX 11 context created successfully");
  }
  else{
    TR_ERROR("Failed to create DirectX 11 context");
  }

  if (win->isFullscreen())
    mSwapchain->SetFullscreenState(TRUE, NULL);

  
  resizeScene(win->getWidth(), win->getHeight());
  //initRendering();  
}

void DXRenderer::killContext(){
  delete mShader;
  mSwapchain->SetFullscreenState(FALSE, NULL);
  SAFE_RELEASE(mSwapchain);
  SAFE_RELEASE(mBackBuffer);
  SAFE_RELEASE(mD3d);
  //ID3D11Debug* dbg;
  //mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&dbg));
  //dbg->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  SAFE_RELEASE(mDevice);
}

static char const * vs_src =
"cbuffer perObject{\n"
"  matrix mvp;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
//"  float4 color: COLOR0;\n"
"  float2 texcoord: TEXCOORD;\n"
//"  float3 normal: NORMAL;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  float4 vPos = float4(inp.pos, 1.0);\n"
"  outp.vPos = mul(mvp, vPos);\n"
"  return outp;\n"
"}\n"
"\n"
;

static char const * fs_src =
"cbuffer perDraw{\n"
"  float4 modColor;\n"
"}\n"
"\n"
"struct PSInput{\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = modColor;\n"
"  return color;\n"
"}\n"
"\n";

void DXRenderer::initRendering(){
  TR_USE(CGE_Renderer_DirectX);
  TR_INFO("Initializing Scene");

  ID3D11Texture2D* backbuffer;
  mSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
  mDevice->CreateRenderTargetView(backbuffer, NULL, &mBackBuffer);
  backbuffer->Release();
  mD3d->OMSetRenderTargets(1, &mBackBuffer, NULL);

  D3D11_RASTERIZER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.FillMode = D3D11_FILL_SOLID;
  desc.CullMode = D3D11_CULL_BACK;
  desc.FrontCounterClockwise = TRUE;
  ID3D11RasterizerState* state;
  mDevice->CreateRasterizerState(&desc, &state);
  mD3d->RSSetState(state);
  state->Release();

  mShader = new CGE::DXShader(VB_POSITION | VB_TEXCOORD);
  mShader->allocUniforms(Shader::VERTEX_SHADER, sizeof(CGE::Matrix));
  mShader->allocUniforms(Shader::FRAGMENT_SHADER, 4 * sizeof(float));
  mShader->addShader(Shader::VERTEX_SHADER, vs_src);
  mShader->addShader(Shader::FRAGMENT_SHADER, fs_src);
  mShader->syncMatrix("mvp", CGE::MVP);
  mShader->activate();
  
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
  XMMATRIX curr = XMLoadFloat4x4(&mMatrix[mMatrixMode]);
  XMStoreFloat4x4(&mMatrix[mMatrixMode], mat*curr);
}

//! set projection
void DXRenderer::projection(float angle, float aspect, float nearplane, float farplane){
  XMStoreFloat4x4(&mMatrix[Projection], XMMatrixPerspectiveFovRH(angle / 180.0f*XM_PI, aspect, nearplane, farplane));
}

void DXRenderer::ortho(float left, float right, float bottom, float top, float nearp, float farp){
  XMStoreFloat4x4(&mMatrix[Projection], XMMatrixOrthographicOffCenterRH(left, right, bottom, top, nearp, farp));
}

//! reset modelview matrix
void DXRenderer::resetModelView(){
  XMStoreFloat4x4(&mMatrix[mMatrixMode], XMMatrixIdentity());
}

//! translate
void DXRenderer::translate(float x, float y, float z){
  XMMATRIX mat = XMMatrixTranslation(x, y, z);
  XMMATRIX curr = XMLoadFloat4x4(&mMatrix[mMatrixMode]);
  XMStoreFloat4x4(&mMatrix[mMatrixMode], mat*curr);
}

//! scale
void DXRenderer::scale(float x, float y, float z){
  XMMATRIX mat = XMMatrixScaling(x, y, z);
  XMMATRIX curr = XMLoadFloat4x4(&mMatrix[mMatrixMode]);
  XMStoreFloat4x4(&mMatrix[mMatrixMode], mat*curr);
}

//! rotate
void DXRenderer::rotate(float angle, float x, float y, float z){
  XMVECTOR axis = XMVectorSet(x, y, z, 0.0f);
  XMMATRIX mat = XMMatrixRotationAxis(axis, angle / 180.0f*XM_PI);
  XMMATRIX curr = XMLoadFloat4x4(&mMatrix[mMatrixMode]);
  XMStoreFloat4x4(&mMatrix[mMatrixMode], mat*curr);
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

void DXRenderer::enableDepthTest(const bool flag){

}
  
//! set color
void DXRenderer::setColor(float r, float g, float b, float a){
  Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
  Shader::getCurrentShader()->uniform(0, r, g, b, a);
  Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
}

void DXRenderer::setColor(const Color* c){
  Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
  Shader::getCurrentShader()->uniform(0, c->r, c->g, c->b, c->a);
  Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
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
  XMMATRIX curr = XMLoadFloat4x4(&mMatrix[mMatrixMode]);
  XMStoreFloat4x4(&mMatrix[mMatrixMode], curr*dxmat);
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
  XMFLOAT4X4 ret;
  if (mt == MVP){
    XMMATRIX p = XMLoadFloat4x4(&mMatrix[Projection]);
    XMMATRIX mv = XMLoadFloat4x4(&mMatrix[Modelview]);
    XMStoreFloat4x4(&ret, mv*p);
  }
  else
    ret = mMatrix[mt];
  //float m[16];
  //memcpy(m, ret.m, 4 * 4* sizeof(float));
  return CGE::Matrix((float*)ret.m);
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

//! switch matrix type
void DXRenderer::switchMatrixStack(MatrixType type){
  mMatrixMode = type;
}

