#ifndef CGE_SHADER_H
#define CGE_SHADER_H

#include <map>

#include <renderer/renderer.h>

namespace CGE{

class Shader{
public:
  enum Type{
    VERTEX_SHADER = 0,
    FRAGMENT_SHADER,
    NUM_SHADERS
  };
  typedef void(*applyLightCB)(Shader* shader, int number, Light const& light, void* userdata);
  Shader();
  void activate() { mOldProg = mCurrShader; use(); mCurrShader = this; }
  void deactivate() { if (mOldProg) mOldProg->use(); else unuse(); mCurrShader = mOldProg; }
  static Shader* getCurrentShader() { return mCurrShader; }
  
  virtual bool addShader(Type shadertype, const char* shaderstring, int stringlen = 0) = 0;
  virtual bool linkShaders() = 0;

  virtual void lockUniforms(Type t, unsigned buffer=0) = 0;
  virtual void unlockUniforms(Type t, unsigned buffer=0) = 0;
  virtual int getUniformLocation(Type t, const char* name) = 0;
  virtual void uniform(int location, int value) = 0;
  virtual void uniform(int location, float value) = 0;
  virtual void uniform(int location, float v0, float v1) = 0;
  virtual void uniform(int location, float v0, float v1, float v2, float v3) = 0;
  virtual void uniform(int location, const CGE::Matrix& mat) = 0;
  virtual void uniform(int location, const CGE::Color& col) = 0;

  void syncMatrix(char const* name, MatrixType type);
  virtual void applyEngineUniforms();
  virtual bool applyMaterial(Material const& mat);
  virtual void applyLight(int number, Light const& light) { if (mApplyLight) mApplyLight(this, number, light, mApplyLightData); }
  void registerLightCB(applyLightCB al, void* data) { mApplyLight = al; mApplyLightData = data; }
protected:
  virtual void use() = 0;
  virtual void unuse() = 0;

  static Shader* mCurrShader;
  Shader* mOldProg;
  std::map<int, MatrixType> mSyncMat;
  applyLightCB mApplyLight;
  void* mApplyLightData;
};

}

#endif
