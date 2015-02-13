#ifndef DX_SHADER_H
#define DX_SHADER_H

#include <renderer/shader.h>
#include <d3d11.h>

struct ID3D11ShaderReflection;

namespace CGE{

class Matrix;

class DXShader : public Shader{
public:
  DXShader();
  ~DXShader();
  bool addShader(Type shadertype, const char* shaderstring, int stringlen = 0);
  bool linkShaders() { return true; }
  void allocUniforms(Type shader, unsigned idx, unsigned size);
  void lockUniforms(Type t, unsigned buffer=0);
  void unlockUniforms(Type t, unsigned buffer=0);
  void uniform(int location, int value);
  void uniform(int location, float value);
  void uniform(int location, float v0, float v1, float v2, float v3);
  void uniform(int location, const CGE::Matrix& mat);
  void uniform(int location, const CGE::Color& col);
  int getUniformLocation(Type t, const char* name);
private:
  void use();
  void unuse();
  bool createAttributes(ID3DBlob* shader);
  int mAttrType;
  ID3D11VertexShader* mVS;
  ID3D11PixelShader* mPS;
  ID3D11InputLayout* mLayout;
  struct ShaderData{
    std::vector<ID3D11Buffer*> constants;
    ID3D11ShaderReflection* refl;
  };
  ShaderData mData[NUM_SHADERS];
  unsigned char* mMappedUBO;
};

}

#endif