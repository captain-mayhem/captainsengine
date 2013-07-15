#include "GL2Shader.h"

#include "io/Tracing.h"

TR_CHANNEL(CGE_Shader)

using namespace CGE;

bool GL2Shader::addShader(GLenum shadertype, const char* shaderstring, int stringlen){
  TR_USE(CGE_Shader);
  if (stringlen == 0)
    stringlen = strlen(shaderstring);
  GLuint shader = glCreateShader(shadertype);
  glShaderSource(shader, 1, &shaderstring, &stringlen);
  glCompileShader(shader);
  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success){
    TR_ERROR("GL2: %s");
  }
  else
    mShaders.push_back(shader);
  GLint len;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  if (len > 0){
    char* log = new char[len];
    glGetShaderInfoLog(shader, len, NULL, log);
    TR_INFO("GL2: %s", log);
    delete [] log;
  }
  return success != 0;
}

bool GL2Shader::linkShaders(){
  TR_USE(CGE_Shader);
  for (unsigned i = 0; i < mShaders.size(); ++i){
    glAttachShader(mProgram, mShaders[i]);
  }
  glLinkProgram(mProgram);
  GLint success = 0;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &success);
  if (!success){
    TR_ERROR("GL2: linking shader failed: ");
  }
  GLint len;
  glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &len);
  if (len > 0){
    char* log = new char[len];
    glGetProgramInfoLog(mProgram, len, NULL, log);
    TR_INFO("GL2: %s", log);
    delete [] log;
  }
  return success != 0;
}

void GL2Shader::deleteShaders(){
  for (unsigned i = 0; i < mShaders.size(); ++i){
    glDeleteShader(mShaders[i]);
  }
  //mShaders.clear();
}