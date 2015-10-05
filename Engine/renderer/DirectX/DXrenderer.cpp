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
  delete mLightShader;
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

static char const * vs_src_unlit =
"cbuffer perObject{\n"
"  matrix mvp;\n"
"  matrix texmat;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
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

static char const * fs_src_unlit =
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  int textureEnabled;\n"
"  float4 modColor;\n"
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

#define NUM_LIGHTS 8

namespace CGE{
  class LightDXShader : public DXShader{
    virtual bool linkShaders(){
      bool ret = DXShader::linkShaders();
      mTexEnabledLoc = getUniformLocation(FRAGMENT_SHADER, "textureEnabled");
      mColorLoc = getUniformLocation(FRAGMENT_SHADER, "matDiffuse");
      mAmbientLoc = getUniformLocation(FRAGMENT_SHADER, "matAmbient");
      mShininessLoc = getUniformLocation(FRAGMENT_SHADER, "matShininess");
      mSpecularLoc = getUniformLocation(FRAGMENT_SHADER, "matSpecular");
      mNumLights = getUniformLocation(FRAGMENT_SHADER, "numLights");
      mLightPosLoc = getUniformLocation(FRAGMENT_SHADER, "lightPos");
      mLightDirLoc = getUniformLocation(FRAGMENT_SHADER, "lightDir");
      mLightCutoffLoc = getUniformLocation(FRAGMENT_SHADER, "lightCutoff");
      mLightColorLoc = getUniformLocation(FRAGMENT_SHADER, "lightColor");
      mLightAttenuationLoc = getUniformLocation(FRAGMENT_SHADER, "lightAttenuation");
      return ret;
    }
    virtual bool applyMaterial(Material const& mat){
      lockUniforms(FRAGMENT_SHADER);
      Color diff = mat.getDiffuse();
      diff.a = mat.getOpacity();
      uniform(mColorLoc, diff);
      uniform(mAmbientLoc, mat.getAmbient());
      uniform(mShininessLoc, mat.getPower());
      uniform(mSpecularLoc, mat.getSpecular());
      Texture const* tex = mat.getDiffuseTex();
      if (!tex)
        tex = mat.getOpacityTex();
      if (tex){
        uniform(mTexEnabledLoc, 1);
        tex->activate();
      }
      else
        uniform(mTexEnabledLoc, 0);
      unlockUniforms(FRAGMENT_SHADER);
      return true;
    }
    virtual void applyLight(int number, Light const& light){
      lockUniforms(FRAGMENT_SHADER, 1);
      Vec4f pos = light.getPosition();
      uniform(mLightPosLoc+number*4*4, pos.x, pos.y, pos.z, pos.w);
      uniform(mLightDirLoc+number*4*4, light.getDirection());
      uniform(mLightCutoffLoc+number*4*4, light.getCutoff() / 180.0f*(float)M_PI);
      uniform(mLightColorLoc + number * 4 * 4, light.getColor());
      uniform(mLightAttenuationLoc + number * 4 * 4, light.getAttenuation());
      unlockUniforms(FRAGMENT_SHADER, 1);
    }

    int mColorLoc;
    int mAmbientLoc;
    int mSpecularLoc;
    int mTexEnabledLoc;
    int mShininessLoc;

    int mNumLights;
    int mLightPosLoc;
    int mLightDirLoc;
    int mLightCutoffLoc;
    int mLightColorLoc;
    int mLightAttenuationLoc;
  };
}

static char const * vs_src_lit =
"cbuffer perObject{\n"
"  matrix mvp;\n"
"  matrix texmat;\n"
"  matrix mvmat;\n"
"  matrix normalmat;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
"  float2 texcoord: TEXCOORD0;\n"
"  float3 normal: NORMAL;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"  float3 vpos : POSITION;\n"
"  float3 vnormal : NORMAL;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  float4 vPos = float4(inp.pos, 1.0);\n"
"  outp.vPos = mul(mvp, vPos);\n"
"  outp.tcoord = mul(texmat, float4(inp.texcoord,0,0));\n"
"  outp.vpos = mul(mvmat, vPos);\n"
"  float4 normal = mul(normalmat, float4(inp.normal, 0));\n"
"  outp.vnormal = normalize(normal);\n"
"  return outp;\n"
"}\n"
"\n"
;

static char const * fs_src_lit =
"#define NUM_LIGHTS 8\n"
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  int textureEnabled;\n"
"  float4 matDiffuse;\n"
"  float4 matAmbient;\n"
"  float4 matSpecular;\n"
"  float matShininess;\n"
"}\n"
"cbuffer perFrame{\n"
"  int numLights;\n"
"  float4 lightPos[NUM_LIGHTS];\n"
"  float3 lightDir[NUM_LIGHTS];\n"
"  float4 lightColor[NUM_LIGHTS];\n"
"  float lightCutoff[NUM_LIGHTS];\n"
"  float lightAttenuation[NUM_LIGHTS];\n"
"};\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"  float3 vpos : POSITION;\n"
"  float3 vnormal : NORMAL;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = matDiffuse;\n"
"  if (textureEnabled != 0){\n"
"    color = color * tex.Sample(sampl, inp.tcoord);\n"
"  }\n"
"  float3 normal = normalize(inp.vnormal);\n"
"  float3 eye = normalize(-inp.vpos);\n"
"  float3 diffuse = float3(0.0,0.0,0.0);\n"
"  float3 specular = float3(0.0,0.0,0.0);\n"
"  float3 ambient = float3(0.0,0.0,0.0);\n"
"  for (int i = 0; i < numLights; ++i){\n"
"    float3 lightvec;\n"
"    float att = 1.0;\n"
"    if (lightPos[i].w == 0.0)\n"
"     lightvec = normalize(lightPos[i].xyz);\n"
"    else{\n"
"      lightvec = normalize(lightPos[i].xyz - inp.vpos);\n"
"      float lightAngle = acos(dot(-lightvec, normalize(lightDir[i])));\n"
"      if (lightAngle > lightCutoff[i]){\n"
"        att = 0.0;\n"
"      }\n"
"      else{\n"
"        float lightDist = length(lightPos[i].xyz-inp.vpos);\n"
"        att = 1.0/(1.0+lightAttenuation[i]*pow(lightDist, 2));\n"
"      }\n"
"    }\n"
"    float3 refl = normalize(reflect(-lightvec, normal));\n"
"    float NL = max(dot(normal,lightvec), 0.0);\n"
"    float spec = 0.0;\n"
"    if (NL > 0.0)\n"
"      spec = pow(max(dot(refl, eye), 0.0), matShininess);\n"
"    \n"
"    diffuse += lightColor[i].rgb*NL*att;\n"
"    specular += lightColor[i].rgb*spec*att;\n"
"    ambient += lightColor[i].rgb*matAmbient.rgb;\n"
"  }\n"
"  float4 finalColor = float4(color.rgb*(ambient + diffuse) + specular*matSpecular.rgb, color.a);\n"
"  return finalColor;\n"
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

  mLightShader = new CGE::LightDXShader();
  mLightShader->addShader(Shader::VERTEX_SHADER, vs_src_lit);
  mLightShader->addShader(Shader::FRAGMENT_SHADER, fs_src_lit);
  mLightShader->linkShaders();
  Material tmp("init");
  mLightShader->applyMaterial(tmp);
  mLightShader->lockUniforms(Shader::FRAGMENT_SHADER);
  mLightShader->uniform(0, 1);
  mLightShader->unlockUniforms(Shader::FRAGMENT_SHADER);
  mLightShader->syncMatrix("mvp", CGE::MVP);
  mLightShader->syncMatrix("texmat", MatTexture);
  mLightShader->syncMatrix("mvmat", Modelview);
  mLightShader->syncMatrix("normalmat", MatNormal);

  mShader = new CGE::DXShader();
  mShader->addShader(Shader::VERTEX_SHADER, vs_src_unlit);
  mShader->addShader(Shader::FRAGMENT_SHADER, fs_src_unlit);
  mShader->linkShaders();
  mShader->lockUniforms(Shader::FRAGMENT_SHADER);
  mShader->uniform(0, 1);
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
void DXRenderer::setClearColor(const Vec4f& color){
  mClearColor[0] = color.x;
  mClearColor[1] = color.y;
  mClearColor[2] = color.z;
  mClearColor[3] = color.w;
}

void DXRenderer::clear(long flags){
  DXRenderTarget* rt = (DXRenderTarget*)DXRenderTarget::getCurrent();
  if (flags & COLORBUFFER){
    for (unsigned i = 0; i < rt->getNumRenderTargets(); ++i){
      mD3d->ClearRenderTargetView(rt->getRenderTarget(i), mClearColor);
    }
  }
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

Shader* DXRenderer::createShader(){
  DXShader* sh = new DXShader();
  return sh;
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
  case BLEND_DST_COLOR:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    break;
  case BLEND_ZERO:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
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
  case BLEND_DST_COLOR:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    break;
  case BLEND_ZERO:
    mBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
    mBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
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
  int texEn = Shader::getCurrentShader()->getUniformLocation(Shader::FRAGMENT_SHADER, "textureEnabled");
  int textureEnabled = flag ? 1 : 0;
  Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
  Shader::getCurrentShader()->uniform(texEn, textureEnabled);
  Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
}

//! enable lighting
void DXRenderer::enableLighting(const bool flag){
  if (flag && Shader::getCurrentShader() != mLightShader){
    Shader::getCurrentShader()->deactivate();
    mLightShader->activate();
  }
  else if (!flag && Shader::getCurrentShader() != mShader){
    Shader::getCurrentShader()->deactivate();
    mShader->activate();
  }
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
  Color modColor = Color(r, g, b, a);
  if (Shader::getCurrentShader() == mLightShader){
    Material tmp("setColor");
    tmp.setDiffuse(modColor);
    Shader::getCurrentShader()->applyMaterial(tmp);
  }
  else{
    Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
    Shader::getCurrentShader()->uniform(4 * sizeof(int), modColor);
    Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
  }
}

void DXRenderer::setColor(const Color* c){
  if (Shader::getCurrentShader() == mLightShader){
    Material tmp("setColor");
    tmp.setDiffuse(*c);
    Shader::getCurrentShader()->applyMaterial(tmp);
  }
  else{
    Shader::getCurrentShader()->lockUniforms(Shader::FRAGMENT_SHADER);
    Shader::getCurrentShader()->uniform(4 * sizeof(int), *c);
    Shader::getCurrentShader()->unlockUniforms(Shader::FRAGMENT_SHADER);
  }
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
  else if (mt == MatNormal){
    XMMATRIX mv = XMLoadFloat4x4((XMFLOAT4X4*)&mMatrix[Modelview]);
    XMMATRIX inv = XMMatrixInverse(NULL, mv);
    XMMATRIX res = XMMatrixTranspose(inv);
    XMStoreFloat4x4(&ret, res);
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

//! switch matrix type
void DXRenderer::switchMatrixStack(MatrixType type){
  mMatrixMode = type;
}

