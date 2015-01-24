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
  Shader();
  void activate() { mOldProg = mCurrShader; use(); mCurrShader = this; }
  void deactivate() { if (mOldProg) mOldProg->use(); else unuse(); mCurrShader = mOldProg; }
  static Shader* getCurrentShader() { return mCurrShader; }

  virtual void lockUniforms(Type t) = 0;
  virtual void unlockUniforms(Type t) = 0;
  virtual int getUniformLocation(Type t, const char* name) = 0;
  virtual void uniform(int location, int value) = 0;
  virtual void uniform(int location, float v0, float v1, float v2, float v3) = 0;
  virtual void uniform(int location, const CGE::Matrix& mat) = 0;

  void syncMatrix(char const* name, MatrixType type);
  void applyEngineUniforms();
protected:
  virtual void use() = 0;
  virtual void unuse() = 0;

  static Shader* mCurrShader;
  Shader* mOldProg;
  std::map<int, MatrixType> mSyncMat;
};

}

#endif