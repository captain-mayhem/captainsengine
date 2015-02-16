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
#include <renderer/shader.h>

namespace CGE{

class GL2Shader : public Shader{
public:
  GL2Shader();
  ~GL2Shader();
  bool addShader(Type shadertype, const char* shaderstring, int stringlen=0);
  bool linkShaders();
  void bindAttribLocation(int location, const char* name) { glBindAttribLocation(mProgram, location, name);}
  int getAttribLocation(const char* name) {return glGetAttribLocation(mProgram, name);}

  void lockUniforms(Type t, unsigned buffer=0) {}
  void unlockUniforms(Type t, unsigned buffer=0) {}
  int getUniformLocation(Type t, const char* name) {return glGetUniformLocation(mProgram, name);}
  void uniform(int location, int value) {glUniform1i(location, value);}
  void uniform(int location, float value) { glUniform1f(location, value); }
  void uniform(int location, float v0, float v1) { glUniform2f(location, v0, v1); }
  void uniform(int location, float v0, float v1, float v2, float v3) { glUniform4f(location, v0, v1, v2, v3); }
  void uniform(int location, const Vec3f& vec) { glUniform3fv(location, 1, vec.data); }
  void uniform(int location, const CGE::Matrix& mat) { glUniformMatrix4fv(location, 1, GL_FALSE, mat.getData()); }
  void uniform(int location, const CGE::Color& col) { glUniform4fv(location, 1, col.array); }
protected:
  virtual void use() { glUseProgram(mProgram); }
  virtual void unuse() { glUseProgram(0); }
  GLuint mProgram;
};

}

#endif
