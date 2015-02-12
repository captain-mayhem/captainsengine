#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#include <windows.h>
#include "../../window/nativeWindows.h"
#include "../../system/engine.h"
#include "DXvertexbuffer.h"
#include "DXtexture.h"
#include "DXrenderer.h"
#include "DXindexbuffer.h"
#include "DXshader.h"
#include "DXrendertarget.h"

using namespace CGE;
using namespace DirectX;

TR_CHANNEL(CGE_Renderer_DirectX)

DXRenderer::DXRenderer(): Renderer(), mDepthState(NULL), mRT(NULL) {
  type_ = DirectX;
  mSwapchain = NULL;
  mD3d = NULL;
  mDevice = NULL;
  for (int i = 0; i < 3; ++i){
#if 1
    XMMATRIX mat = XMMatrixIdentity();
    XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[i], mat);
#else
    mMatrix[i] = Matrix(Matrix::Identity);
#endif
  }
  mMatrixMode = Modelview;
  mPSUniforms.mModColor = Color(1.0, 1.0, 1.0, 1.0);
  mPSUniforms.mTextureEnabled = 0;
  mBlendState = NULL;
  mBlendDesc.AlphaToCoverageEnable = FALSE;
  mBlendDesc.IndependentBlendEnable = FALSE;
  mBlendDesc.RenderTarget[0].BlendEnable = TRUE;
  mBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  mBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  mBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

  ZeroMemory(&mDepthDesc, sizeof(mDepthDesc));
  mDepthDesc.DepthEnable = TRUE;
  mDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  mDepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
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
  scd.SampleDesc.Count = 1;
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

  mRT = new DXRenderTarget(mSwapchain);
  mRT->addRenderbuffer(Texture::DEPTH);
  mRT->bind();
  RenderTarget::mCurrTarget = mRT;

  enableDepthTest(true);

  if (win->isFullscreen())
    mSwapchain->SetFullscreenState(TRUE, NULL);

  
  resizeScene(win->getWidth(), win->getHeight());
  //initRendering();  
}

void DXRenderer::killContext(){
  delete mShader;
  mSwapchain->SetFullscreenState(FALSE, NULL);
  SAFE_RELEASE(mDepthState);
  SAFE_RELEASE(mBlendState);
  SAFE_RELEASE(mSwapchain);
  delete mRT;
  SAFE_RELEASE(mD3d);
  //ID3D11Debug* dbg;
  //mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&dbg));
  //dbg->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  SAFE_RELEASE(mDevice);
}

static char const * vs_src =
"cbuffer perObject{\n"
"  matrix mvp;\n"
"  matrix texmat;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
//"  float4 color: COLOR0;\n"
"  float2 texcoord: TEXCOORD0;\n"
"  float3 normal: NORMAL;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  float4 vPos = float4(inp.pos, 1.0);\n"
"  outp.vPos = mul(mvp, vPos);\n"
"  outp.tcoord = mul(texmat, float4(inp.texcoord,0,0));\n"
"  return outp;\n"
"}\n"
"\n"
;

static char const * fs_src =
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float4 modColor;\n"
"  int textureEnabled;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = modColor;\n"
"  if (textureEnabled != 0){\n"
"    color = color * tex.Sample(sampl, inp.tcoord);\n"
"  }\n"
"  return color;\n"
"}\n"
"\n";

void DXRenderer::initRendering(){
  TR_USE(CGE_Renderer_DirectX);
  TR_INFO("Initializing Scene");

  D3D11_RASTERIZER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.FillMode = D3D11_FILL_SOLID;
  desc.CullMode = D3D11_CULL_BACK;
  desc.DepthClipEnable = TRUE;
  desc.FrontCounterClockwise = TRUE;
  ID3D11RasterizerState* state;
  mDevice->CreateRasterizerState(&desc, &state);
  mD3d->RSSetState(state);
  state->Release();

  mShader = new CGE::DXShader();
  mShader->addShader(Shader::VERTEX_SHADER, vs_src);
  mShader->addShader(Shader::FRAGMENT_SHADER, fs_src);
  mShader->lockUniforms(Shader::FRAGMENT_SHADER);
  mShader->uniform(4 * sizeof(float), 1);
  mShader->unlockUniforms(Shader::FRAGMENT_SHADER);
  mShader->syncMatrix("mvp", CGE::MVP);
  mShader->syncMatrix("texmat", MatTexture);
  mShader->activate();

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
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
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
  DXRenderTarget* rt = (DXRenderTarget*)DXRenderTarget::getCurrent();
  if (flags & COLORBUFFER)
    mD3d->ClearRenderTargetView(rt->getRenderTargets(), mClearColor);
  if (flags & ZBUFFER)
    mD3d->ClearDepthStencilView(rt->getDepthStencil(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

VertexBuffer* DXRenderer::createVertexBuffer(){
  return new DXVertexBuffer();
}

IndexBuffer* DXRenderer::createIndexBuffer(IndexBuffer::Type t, uint32 size){
  return new DXIndexBuffer(t,size);
}

Texture* DXRenderer::createTexture(Image* img, Texture::Format fmt){
  DXTexture* tex = new DXTexture();
  if (tex->createFromImage(img, fmt))
    return tex;
  delete tex;
  return NULL;
}

RenderTarget* DXRenderer::createRenderTarget(unsigned width, unsigned height){
  DXRenderTarget* rt = new DXRenderTarget(width, height);
  return rt;
}


//! set lookAt
void DXRenderer::lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up){
#if 1
  XMVECTOR eye = XMVectorSet(position.x, position.y, position.z, 1.0f);
  XMVECTOR l = XMVectorSet(look.x, look.y, look.z, 1.0f);
  XMVECTOR u = XMVectorSet(up.x, up.y, up.z, 0.0f);
  XMMATRIX mat = XMMatrixLookAtRH(eye, l, u);
  XMMATRIX curr = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode]);
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode], mat*curr);
#else
  Vec3f forward = look - position;
  forward.normalize();
  Vec3f side = forward.cross(up).normalized();
  Vec3f up_new = side.cross(forward);
  Matrix mat = Matrix(side, up_new, forward*-1, Vec3f()/*eye*1*/)*Matrix(Matrix::Translation, position*-1);
  multiplyMatrix(mat);
#endif
}

//! set projection
void DXRenderer::projection(float angle, float aspect, float nearplane, float farplane){
#if 1
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[Projection], XMMatrixPerspectiveFovRH(angle / 180.0f*XM_PI, aspect, nearplane, farplane));
#else
  float ymax = nearplane * (float)tan(angle * 3.1415962f / 360.0);
  float ymin = -ymax;
  float xmin = ymin * aspect;
  float xmax = ymax * aspect;
  mMatrix[Projection] = CGE::Matrix(CGE::Matrix::Perspective, xmin, xmax, ymin, ymax, nearplane, farplane);
#endif
}

void DXRenderer::ortho(float left, float right, float bottom, float top, float nearp, float farp){
#if 1
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[Projection], XMMatrixOrthographicOffCenterRH(left, right, bottom, top, nearp, farp));
#else
  mMatrix[Projection] = CGE::Matrix(CGE::Matrix::Ortho, left, right, bottom, top, nearp, farp);
#endif
}

//! reset modelview matrix
void DXRenderer::resetModelView(){
#if 1
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode], XMMatrixIdentity());
#else
  mMatrix[mMatrixMode] = Matrix(Matrix::Identity);
#endif
}

//! translate
void DXRenderer::translate(float x, float y, float z){
#if 1
  XMMATRIX mat = XMMatrixTranslation(x, y, z);
  XMMATRIX curr = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode]);
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode], mat*curr);
#else
  mMatrix[mMatrixMode] *= Matrix(Matrix::Translation, Vec3f(x, y, z));
#endif
}

//! scale
void DXRenderer::scale(float x, float y, float z){
#if 1
  XMMATRIX mat = XMMatrixScaling(x, y, z);
  XMMATRIX curr = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode]);
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode], mat*curr);
#else
  mMatrix[mMatrixMode] *= Matrix(Matrix::Scale, Vec3f(x, y, z));
#endif
}

//! rotate
void DXRenderer::rotate(float angle, float x, float y, float z){
#if 1
  XMVECTOR axis = XMVectorSet(x, y, z, 0.0f);
  XMMATRIX mat = XMMatrixRotationAxis(axis, angle / 180.0f*XM_PI);
  XMMATRIX curr = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode]);
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode], mat*curr);
#else
  mMatrix[mMatrixMode] *= Matrix(Matrix::Rotation, Vec3f(x, y, z), angle / 180 * (float)M_PI);
#endif
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
  BOOL curr = mBlendDesc.RenderTarget[0].BlendEnable;
  mBlendDesc.RenderTarget[0].BlendEnable = TRUE;
  switch (src){
  case BLEND_ONE:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    break;
  case BLEND_SRC_ALPHA:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    break;
  case BLEND_ONE_MINUS_SRC_ALPHA:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_ALPHA;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    break;
  }
  switch (dest){
  case BLEND_ONE:
    mBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    mBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    break;
  case BLEND_SRC_ALPHA:
    mBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
    mBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    break;
  case BLEND_ONE_MINUS_SRC_ALPHA:
    mBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    mBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    break;
  }
  SAFE_RELEASE(mBlendState);
  mDevice->CreateBlendState(&mBlendDesc, &mBlendState);
  if (curr)
    mD3d->OMSetBlendState(mBlendState, NULL, 0xffffffff);
  mBlendDesc.RenderTarget[0].BlendEnable = curr;
}

//! enable blending
void DXRenderer::enableBlend(const bool flag){
  mBlendDesc.RenderTarget[0].BlendEnable = flag ? TRUE : FALSE;
  if (flag)
    mD3d->OMSetBlendState(mBlendState, NULL, 0xffffffff);
  else
    mD3d->OMSetBlendState(NULL, NULL, 0xffffffff);
}

void DXRenderer::enableBackFaceCulling(const bool flag){
  /*D3DCULL cull = D3DCULL_NONE;
  if (flag)
    cull = D3DCULL_CW;
  device_->SetRenderState(D3DRS_CULLMODE, cull);*/
}

//! enable texturing
void DXRenderer::enableTexturing(const bool flag){
  mPSUniforms.mTextureEnabled = flag ? 1 : 0;
  Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
  Shader::getCurrentShader()->uniform(0, mPSUniforms.mModColor.r, mPSUniforms.mModColor.g, mPSUniforms.mModColor.b, mPSUniforms.mModColor.a);
  Shader::getCurrentShader()->uniform(sizeof(Color), mPSUniforms.mTextureEnabled);
  Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
}

//! enable lighting
void DXRenderer::enableLighting(const bool flag){
  //device_->SetRenderState(D3DRS_LIGHTING, flag);
}

void DXRenderer::enableDepthTest(const bool flag){
  SAFE_RELEASE(mDepthState);
  mDepthDesc.DepthEnable = flag ? TRUE : FALSE;
  mDevice->CreateDepthStencilState(&mDepthDesc, &mDepthState);
  mD3d->OMSetDepthStencilState(mDepthState, 0);
}

void DXRenderer::enableDepthWrite(bool flag){
  SAFE_RELEASE(mDepthState);
  mDepthDesc.DepthWriteMask = flag ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
  mDevice->CreateDepthStencilState(&mDepthDesc, &mDepthState);
  mD3d->OMSetDepthStencilState(mDepthState, 0);
}
  
//! set color
void DXRenderer::setColor(float r, float g, float b, float a){
  mPSUniforms.mModColor = Color(r, g, b, a);
  Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
  Shader::getCurrentShader()->uniform(0, mPSUniforms.mModColor.r, mPSUniforms.mModColor.g, mPSUniforms.mModColor.b, mPSUniforms.mModColor.a);
  Shader::getCurrentShader()->uniform(sizeof(Color), mPSUniforms.mTextureEnabled);
  Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
}

void DXRenderer::setColor(const Color* c){
  mPSUniforms.mModColor = *c;
  Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
  Shader::getCurrentShader()->uniform(0, mPSUniforms.mModColor.r, mPSUniforms.mModColor.g, mPSUniforms.mModColor.b, mPSUniforms.mModColor.a);
  Shader::getCurrentShader()->uniform(sizeof(Color), mPSUniforms.mTextureEnabled);
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
#if 1
  XMMATRIX dxmat = XMMatrixSet(mat.getData()[0], mat.getData()[1], mat.getData()[2], mat.getData()[3],
    mat.getData()[4], mat.getData()[5], mat.getData()[6], mat.getData()[7],
    mat.getData()[8], mat.getData()[9], mat.getData()[10], mat.getData()[11],
    mat.getData()[12], mat.getData()[13], mat.getData()[14], mat.getData()[15]);
  XMMATRIX curr = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode]);
  XMStoreFloat4x4((XMFLOAT4X4*)&mMatrix[mMatrixMode], dxmat*curr);
#else
  mMatrix[mMatrixMode] *= mat;
#endif
}

//! set material
/*void DXRenderer::setMaterial(const Material& mat){
  if (mat.getDiffuse().a != 1.0f)
    enableBlend(true);
  else
    enableBlend(false);
  setColor(&mat.getDiffuse());
  Texture const* tex = mat.getDiffuseTex();
  if (tex){
    enableTexturing(true);
    tex->activate();
  }
  else
    enableTexturing(false);
}*/


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
#if 1
  XMFLOAT4X4 ret;
  if (mt == MVP){
    XMMATRIX p = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[Projection]);
    XMMATRIX mv = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[Modelview]);
    XMStoreFloat4x4(&ret, mv*p);
  }
  else
    ret = *(XMFLOAT4X4*)&mMatrix[mt];
  //float m[16];
  //memcpy(m, ret.m, 4 * 4* sizeof(float));
  return *(Matrix*)&ret;//CGE::Matrix((float*)ret.m);
#else
  if (mt == MVP){
    return mMatrix[Projection] * mMatrix[Modelview];
  }
  return mMatrix[mt];
#endif
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

//! switch matrix type
void DXRenderer::switchMatrixStack(MatrixType type){
  mMatrixMode = type;
}

