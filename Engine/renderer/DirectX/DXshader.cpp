#include "DXshader.h"
#include <system/engine.h>
#include <renderer/DirectX/DXrenderer.h>
#include <d3dcompiler.h>

using namespace CGE;

TR_CHANNEL(CGE_Shader_DX);

DXShader::DXShader() : mVS(NULL), mPS(NULL), mLayout(NULL), mAttrType(0), mMappedUBO(NULL){
  for (int i = 0; i < Shader::NUM_SHADERS; ++i){
    mData[i].refl = NULL;
  }
}

DXShader::~DXShader(){
  SAFE_RELEASE(mVS);
  SAFE_RELEASE(mPS);
  SAFE_RELEASE(mLayout);
  for (int i = 0; i < Shader::NUM_SHADERS; ++i){
    for (unsigned j = 0; j < mData[i].constants.size(); ++j){
      SAFE_RELEASE(mData[i].constants[j]);
    }
    SAFE_RELEASE(mData[i].refl);
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
  }
  else if (shadertype == FRAGMENT_SHADER)
    dev->CreatePixelShader(shader->GetBufferPointer(), shader->GetBufferSize(), NULL, &mPS);

  D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&mData[shadertype].refl);
  if (mData[shadertype].refl){
    D3D11_SHADER_DESC sdesc;
    mData[shadertype].refl->GetDesc(&sdesc);

    mData[shadertype].constants.resize(sdesc.ConstantBuffers);
    for (unsigned i = 0; i < sdesc.ConstantBuffers; ++i){
      ID3D11ShaderReflectionConstantBuffer* cbr = mData[shadertype].refl->GetConstantBufferByIndex(i);
      if (cbr){
        D3D11_SHADER_BUFFER_DESC desc;
        cbr->GetDesc(&desc);
        allocUniforms(shadertype, i, desc.Size);
      }
    }
    if (shadertype == VERTEX_SHADER){
      for (unsigned i = 0; i < sdesc.InputParameters; ++i){
        D3D11_SIGNATURE_PARAMETER_DESC pdesc;
        mData[shadertype].refl->GetInputParameterDesc(i, &pdesc);
        if (strcmp(pdesc.SemanticName, "POSITION") == 0)
          mAttrType |= VB_POSITION;
        else if (strcmp(pdesc.SemanticName, "TEXCOORD") == 0)
          mAttrType |= VB_TEXCOORD;
        else if (strcmp(pdesc.SemanticName, "NORMAL") == 0)
          mAttrType |= VB_NORMAL;
        else if (strcmp(pdesc.SemanticName, "COLOR") == 0)
          mAttrType |= VB_COLOR;
        else
          TR_WARN("%s unknown, cannot deduce Vertex Buffer layout", pdesc.SemanticName);
      }
      createAttributes(shader);
    }
  }

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

void DXShader::allocUniforms(Type shader, unsigned idx, unsigned size){
  D3D11_BUFFER_DESC desc;
  desc.ByteWidth = size;
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = malloc(size);
  data.SysMemPitch = size;
  memset((void*)data.pSysMem, 0, size);
  dev->CreateBuffer(&desc, &data, &mData[shader].constants[idx]);
  free((void*)data.pSysMem);
}

void DXShader::use(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->VSSetShader(mVS, 0, 0);
  ctx->VSSetConstantBuffers(0, mData[VERTEX_SHADER].constants.size(), mData[VERTEX_SHADER].constants.data());
  ctx->PSSetShader(mPS, 0, 0);
  ctx->PSSetConstantBuffers(0, mData[FRAGMENT_SHADER].constants.size(), mData[FRAGMENT_SHADER].constants.data());
  ctx->IASetInputLayout(mLayout);
}

void DXShader::unuse(){
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->VSSetShader(NULL, 0, 0);
  ctx->PSSetShader(NULL, 0, 0);
  ctx->VSSetConstantBuffers(0, 0, NULL);
  ctx->IASetInputLayout(NULL);
}

void DXShader::lockUniforms(Type t, unsigned buffer){
  if (mData[t].constants.size() <= buffer || mData[t].constants[buffer] == NULL)
    return;
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  D3D11_MAPPED_SUBRESOURCE mr;
  ctx->Map(mData[t].constants[buffer], 0, D3D11_MAP_WRITE_DISCARD, NULL, &mr);
  mMappedUBO = (unsigned char*)mr.pData;
}

void DXShader::unlockUniforms(Type t, unsigned buffer){
  if (mData[t].constants.size() <= buffer || mData[t].constants[buffer] == NULL)
    return;
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->Unmap(mData[t].constants[buffer], 0);
}

#define UCHECK() if (!mMappedUBO || location < 0) return;

void DXShader::uniform(int location, int value){
  UCHECK();
  int* iloc = (int*)(mMappedUBO + location);
  *iloc = value;
}

void DXShader::uniform(int location, float value){
  UCHECK();
  float* floc = (float*)(mMappedUBO + location);
  *floc = value;
}

void DXShader::uniform(int location, float v0, float v1, float v2, float v3){
  UCHECK();
  float* flt = (float*)(mMappedUBO+location);
  flt[0] = v0;
  flt[1] = v1;
  flt[2] = v2;
  flt[3] = v3;
}

void DXShader::uniform(int location, const Vec3f& vec){
  UCHECK();
  memcpy(mMappedUBO + location, vec.data, sizeof(vec));
}

void DXShader::uniform(int location, const CGE::Matrix& mat){
  UCHECK();
  memcpy(mMappedUBO+location, mat.getData(), sizeof(mat));
}

void DXShader::uniform(int location, const CGE::Color& col){
  UCHECK();
  memcpy(mMappedUBO + location, col.array, sizeof(col));
}

int DXShader::getUniformLocation(Type t, const char* name){
  if (!mData[t].refl)
    return -1;
  ID3D11ShaderReflectionVariable* var = mData[t].refl->GetVariableByName(name);
  if (var == NULL)
    return -1;
  D3D11_SHADER_VARIABLE_DESC d;
  var->GetDesc(&d);
  return d.StartOffset;
}

