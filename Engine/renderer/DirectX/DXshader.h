#ifndef DX_SHADER_H
#define DX_SHADER_H

#include <renderer/shader.h>
#include <d3d11.h>

namespace CGE{

class Matrix;

class DXShader : public Shader{
public:
  DXShader(int attrType);
  ~DXShader();
  bool addShader(Type shadertype, const char* shaderstring, int stringlen = 0);
  void allocUniforms(Type shader, unsigned size);
  void lockUniforms(Type t);
  void unlockUniforms(Type t);
  void uniform(int location, float v0, float v1, float v2, float v3);
  void uniform(int location, const CGE::Matrix& mat);
  int getUniformLocation(const char* name);
private:
  void use();
  void unuse();
  bool createAttributes(ID3DBlob* shader);
  int mAttrType;
  ID3D11VertexShader* mVS;
  ID3D11PixelShader* mPS;
  ID3D11InputLayout* mLayout;
  ID3D11Buffer* mConstants[NUM_SHADERS];
  unsigned char* mMappedUBO;
};

}

#endif