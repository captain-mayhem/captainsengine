#ifndef GL2_SHADER_H
#define GL2_SHADER_H

#include <GL/glew.h>

#include <vector>

namespace CGE{

class GL2Shader{
public:
  GL2Shader(){
    mProgram = glCreateProgram();
  }
  ~GL2Shader(){
    deleteShaders();
    glDeleteProgram(mProgram);
  }
  bool addShader(GLenum shadertype, const char* shaderstring, int stringlen=0);
  bool linkShaders();
  void deleteShaders();
  void activate() {glGetIntegerv(GL_CURRENT_PROGRAM, &mOldProg); glUseProgram(mProgram);}
  void deactivate() {glUseProgram(mOldProg);}
  void bindAttribLocation(int location, const char* name) { glBindAttribLocation(mProgram, location, name);}
  int getAttribLocation(const char* name) {return glGetAttribLocation(mProgram, name);}
  int getUniformLocation(const char* name) {return glGetUniformLocation(mProgram, name);}
  void uniform(int location, int value) {glUniform1i(location, value);}
  void uniform(int location, float value) {glUniform1f(location, value);}
  void uniform(int location, float v0, float v1) {glUniform2f(location, v0, v1);}
  void uniform(int location, float v0, float v1, float v2, float v3) {glUniform4f(location, v0, v1, v2, v3);}
protected:
  GLuint mProgram;
  GLint mOldProg;
  std::vector<GLuint> mShaders;
};

}

#endif
