#include "GL2Shader.h"

#include <cstring>

#include "io/Tracing.h"
#include "system/engine.h"
#include "renderer/renderer.h"

TR_CHANNEL(CGE_Shader)

using namespace CGE;

GL2Shader::GL2Shader(){
  mProgram = glCreateProgram();
  bindAttribLocation(0, "pos");
  bindAttribLocation(1, "color");
  bindAttribLocation(2, "texcoord");
  bindAttribLocation(3, "normal");
  bindAttribLocation(4, "texcoord2");
}

GL2Shader::~GL2Shader(){
  glDeleteProgram(mProgram);
}

bool GL2Shader::addShader(Type shadertype, const char* shaderstring, int stringlen){
  TR_USE(CGE_Shader);
  
  std::string shadercode;
  if (stringlen == 0)
    shadercode = shaderstring;
  else
    shadercode = std::string(shaderstring, stringlen);
  size_t idx = shadercode.find("@GLSL");
  if (idx != std::string::npos){
    size_t endidx = shadercode.find("@", idx + 5);
    shadercode = shadercode.substr(idx + 5, endidx - idx - 5);
  }

  GLenum type;
  if (shadertype == VERTEX_SHADER)
    type = GL_VERTEX_SHADER;
  else if (shadertype == FRAGMENT_SHADER)
    type = GL_FRAGMENT_SHADER;
  GLuint shader = glCreateShader(type);
  GLchar const* ss = shadercode.c_str();
  GLint slen = (GLint)shadercode.size();
  glShaderSource(shader, 1, &ss, &slen);
  glCompileShader(shader);
  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success){
    glAttachShader(mProgram, shader);
  }
  else{
    GLint len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0){
      char* log = new char[len];
      glGetShaderInfoLog(shader, len, NULL, log);
      TR_ERROR("GL2: %s\nshader was\n%s", log, shadercode.c_str());
      delete [] log;
    }
  }
  glDeleteShader(shader);
  return success != 0;
}

bool GL2Shader::linkShaders(){
  TR_USE(CGE_Shader);
  glLinkProgram(mProgram);
  GLint success = 0;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &success);
  if (!success){
    TR_ERROR("GL2: linking shader failed: ");
    GLint len;
    glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &len);
    if (len > 0){
      char* log = new char[len];
      glGetProgramInfoLog(mProgram, len, NULL, log);
      TR_ERROR("GL2: %s", log);
      delete [] log;
    }
  }
  return success != 0;
}
