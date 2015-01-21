#include "DXshader.h"
#include <system/engine.h>
#include <renderer/DirectX/DXrenderer.h>
#include <d3dcompiler.h>

using namespace CGE;

TR_CHANNEL(CGE_Shader_DX);

DXShader::DXShader(int attrType) : mVS(NULL), mPS(NULL), mLayout(NULL), mAttrType(attrType), mMappedUBO(NULL){
  for (int i = 0; i < Shader::NUM_SHADERS; ++i){
    mConstants[i] = NULL;
  }
}

DXShader::~DXShader(){
  SAFE_RELEASE(mVS);
  SAFE_RELEASE(mPS);
  SAFE_RELEASE(mLayout);
  for (int i = 0; i < Shader::NUM_SHADERS; ++i){
    SAFE_RELEASE(mConstants[i]);
  }
}

bool DXShader::addShader(Type shadertype, const char* shaderstring, int stringlen){
  TR_USE(CGE_Shader_DX);
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  if (stringlen == 0)
    stringlen = strlen(shaderstring);
  char* type;
  if (shadertype == VERTEX_SHADER)
    type = "vs_5_0";
  else if (shadertype == FRAGMENT_SHADER)
    type = "ps_5_0";
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  flags |= D3DCOMPILE_DEBUG;
#endif
  ID3DBlob* shader = NULL;
  ID3DBlob* errors = NULL;
  HRESULT res = D3DCompile(shaderstring, stringlen, NULL, NULL, NULL, "main", type, flags, 0, &shader, &errors);
  if (FAILED(res)){
    if (errors){
      TR_ERROR("DX: %s\nshader was\n%s", errors->GetBufferPointer(), shaderstring);
      errors->Release();
    }
    if (shader)
      shader->Release();
    return false;
  }
  if (shadertype == VERTEX_SHADER){
    dev->CreateVertexShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &mVS);
    createAttributes(shader);
  }
  else if (shadertype == FRAGMENT_SHADER)
    dev->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &mPS);
  ID3D11ShaderReflection* refl;
  D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&refl);
  //D3D11_SHADER_INPUT_BIND_DESC bind;
  //refl->GetResourceBindingDescByName("mvp", &bind);
  ID3D11ShaderReflectionVariable* var = refl->GetVariableByName("MVP");
  D3D11_SHADER_VARIABLE_DESC d;
  var->GetDesc(&d);
  refl->Release();
  shader->Release();
  return true;
}

bool DXShader::createAttributes(ID3DBlob* shader){
  if (mVS == NULL)
    return false;
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  D3D11_INPUT_ELEMENT_DESC edesc[4];
  int arr = 0;
  int offset = 0;
  if (mAttrType & VB_POSITION){
    edesc[arr].SemanticName = "POSITION";
    edesc[arr].SemanticIndex = 0;
    edesc[arr].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    edesc[arr].InputSlot = 0;
    edesc[arr].AlignedByteOffset = offset;
    edesc[arr].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    edesc[arr].InstanceDataStepRate = 0;
    ++arr;
    offset += 3 * sizeof(float);
  }
  if (mAttrType & VB_COLOR){
    edesc[arr].SemanticName = "COLOR";
    edesc[arr].SemanticIndex = 0;
    edesc[arr].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    edesc[arr].InputSlot = 0;
    edesc[arr].AlignedByteOffset = offset;
    edesc[arr].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    edesc[arr].InstanceDataStepRate = 0;
    ++arr;
    offset += 4 * sizeof(char);
  }
  if (mAttrType & VB_TEXCOORD){
    edesc[arr].SemanticName = "TEXCOORD";
    edesc[arr].SemanticIndex = 0;
    edesc[arr].Format = DXGI_FORMAT_R32G32_FLOAT;
    edesc[arr].InputSlot = 0;
    edesc[arr].AlignedByteOffset = offset;
    edesc[arr].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    edesc[arr].InstanceDataStepRate = 0;
    ++arr;
    offset += 2 * sizeof(float);
  }
  if (mAttrType & VB_NORMAL){
    edesc[arr].SemanticName = "NORMAL";
    edesc[arr].SemanticIndex = 0;
    edesc[arr].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    edesc[arr].InputSlot = 0;
    edesc[arr].AlignedByteOffset = offset;
    edesc[arr].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    edesc[arr].InstanceDataStepRate = 0;
    ++arr;
    offset += 3 * sizeof(float);
  }
  dev->CreateInputLayout(edesc, arr, shader->GetBufferPointer(), shader->GetBufferSize(), &mLayout);
  return true;
}

void DXShader::allocUniforms(Type shader, unsigned size){
  D3D11_BUFFER_DESC desc;
  desc.ByteWidth = size;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  dev->CreateBuffer(&desc, NULL, &mConstants[shader]);
}

void DXShader::use(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->VSSetShader(mVS, 0, 0);
  ctx->VSSetConstantBuffers(0, 1, &mConstants[Shader::VERTEX_SHADER]);
  ctx->PSSetShader(mPS, 0, 0);
  ctx->PSSetConstantBuffers(0, 1, &mConstants[Shader::FRAGMENT_SHADER]);
  ctx->IASetInputLayout(mLayout);
}

void DXShader::unuse(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->VSSetShader(NULL, 0, 0);
  ctx->PSSetShader(NULL, 0, 0);
  ctx->VSSetConstantBuffers(0, 0, NULL);
  ctx->IASetInputLayout(NULL);
}

void DXShader::lockUniforms(Type t){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  D3D11_MAPPED_SUBRESOURCE mr;
  ctx->Map(mConstants[t], 0, D3D11_MAP_WRITE_DISCARD, NULL, &mr);
  mMappedUBO = (unsigned char*)mr.pData;
}

void DXShader::unlockUniforms(Type t){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->Unmap(mConstants[t], 0);
}

void DXShader::uniform(int location, float v0, float v1, float v2, float v3){
  float* flt = (float*)(mMappedUBO+location);
  flt[0] = v0;
  flt[1] = v1;
  flt[2] = v2;
  flt[3] = v3;
}

void DXShader::uniform(int location, const CGE::Matrix& mat){
  TR_USE(CGE_Shader_DX);
  if (mMappedUBO == NULL){
    TR_ERROR("Attempt to set uniform on non-locked UBO");
    return;
  }
  memcpy(mMappedUBO+location, mat.getData(), sizeof(mat));
}

int DXShader::getUniformLocation(const char* name){
  return 0;
}

