#ifndef DX_SHADER_H
#define DX_SHADER_H

#include <renderer/shader.h>
#include <d3d11.h>

namespace CGE{

class DXShader : public Shader{
public:
  DXShader(int attrType);
  ~DXShader();
  bool addShader(Type shadertype, const char* shaderstring, int stringlen = 0);
  void activate();
private:
  bool createAttributes(ID3DBlob* shader);
  int mAttrType;
  ID3D11VertexShader* mVS;
  ID3D11PixelShader* mPS;
  ID3D11InputLayout* mLayout;
};

}

#endif