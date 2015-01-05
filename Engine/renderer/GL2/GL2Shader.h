#ifndef GL2_SHADER_H
#define GL2_SHADER_H

#ifndef QNX
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif
#include <map>
#include <math/matrix.h>
#include <renderer/renderer.h>

namespace CGE{

class GL2Shader{
public:
  GL2Shader();
  ~GL2Shader();
  bool addShader(GLenum shadertype, const char* shaderstring, int stringlen=0);
  bool linkShaders();
  void activate() { mOldProg = mCurrShader; glUseProgram(mProgram); mCurrShader = this; }
  void deactivate() { glUseProgram(mOldProg?mOldProg->mProgram:0); mCurrShader = mOldProg; }
  void bindAttribLocation(int location, const char* name) { glBindAttribLocation(mProgram, location, name);}
  int getAttribLocation(const char* name) {return glGetAttribLocation(mProgram, name);}
  int getUniformLocation(const char* name) {return glGetUniformLocation(mProgram, name);}
  void syncMatrix(char const* name, MatrixType type);
  void applyEngineUniforms();
  static void uniform(int location, int value) {glUniform1i(location, value);}
  static void uniform(int location, float value) { glUniform1f(location, value); }
  static void uniform(int location, float v0, float v1) { glUniform2f(location, v0, v1); }
  static void uniform(int location, float v0, float v1, float v2, float v3) { glUniform4f(location, v0, v1, v2, v3); }
  static void uniform(int location, const CGE::Matrix& mat) { glUniformMatrix4fv(location, 1, GL_FALSE, mat.getData()); }
  static GL2Shader* getCurrentShader() { return mCurrShader; }
protected:
  static GL2Shader* mCurrShader;
  GLuint mProgram;
  GL2Shader* mOldProg;
  std::map<int, MatrixType> mSyncMat;
};

}

#endif
