#ifndef GL2_SHADER_H
#define GL2_SHADER_H

#include <cge_export.h>

#if !defined(ANDROID)
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif
#include <map>
#include <math/matrix.h>
#include <renderer/renderer.h>
#include <renderer/shader.h>

namespace CGE{

class CGE_API GL2Shader : public Shader{
public:
  GL2Shader();
  ~GL2Shader();
  bool addShader(Type shadertype, const char* shaderstring, int stringlen=0);
  bool linkShaders();
  void bindAttribLocation(int location, const char* name);
  int getAttribLocation(const char* name);

  void lockUniforms(Type t, unsigned buffer=0) {}
  void unlockUniforms(Type t, unsigned buffer=0) {}
  int getUniformLocation(Type t, const char* name);
  void uniform(int location, int value);
  void uniform(int location, float value);
  void uniform(int location, float v0, float v1);
  void uniform(int location, float v0, float v1, float v2, float v3);
  void uniform(int location, const Vec3f& vec);
  void uniform(int location, const CGE::Matrix& mat);
  void uniform(int location, const CGE::Color& col);
protected:
  virtual void use();
  virtual void unuse();
  GLuint mProgram;
};

}

#endif