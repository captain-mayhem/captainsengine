#ifndef GL2_SHADER_H
#define GL2_SHADER_H

#ifndef QNX
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif
#include <math/matrix.h>

namespace CGE{

class GL2Shader{
public:
  GL2Shader();
  ~GL2Shader();
  bool addShader(GLenum shadertype, const char* shaderstring, int stringlen=0);
  bool linkShaders();
  void activate() {glGetIntegerv(GL_CURRENT_PROGRAM, &mOldProg); glUseProgram(mProgram);}
  void deactivate() {glUseProgram(mOldProg);}
  void bindAttribLocation(int location, const char* name) { glBindAttribLocation(mProgram, location, name);}
  int getAttribLocation(const char* name) {return glGetAttribLocation(mProgram, name);}
  int getUniformLocation(const char* name) {return glGetUniformLocation(mProgram, name);}
  static void uniform(int location, int value) {glUniform1i(location, value);}
  static void uniform(int location, float value) { glUniform1f(location, value); }
  static void uniform(int location, float v0, float v1) { glUniform2f(location, v0, v1); }
  static void uniform(int location, float v0, float v1, float v2, float v3) { glUniform4f(location, v0, v1, v2, v3); }
  static void uniform(int location, const CGE::Matrix& mat) { glUniformMatrix4fv(location, 1, GL_FALSE, mat.getData()); }
protected:
  GLuint mProgram;
  GLint mOldProg;
};

}

#endif
