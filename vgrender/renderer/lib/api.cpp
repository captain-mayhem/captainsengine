#include <GLES3/gl3.h>
#undef GL_APICALL
#define GL_APICALL /*__declspec(dllexport)*/
#include "api.h"

GL_APICALL void GL_APIENTRY GL(ClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){
  VR_CONTEXT->setClearColor((unsigned char)(red*255), 
    (unsigned char)(green*255), 
    (unsigned char)(blue*255), 
    (unsigned char)(alpha*255));
}

GL_APICALL void GL_APIENTRY GL(Clear) (GLbitfield mask){
  if (mask & GL_COLOR_BUFFER_BIT)
    VR_CONTEXT->getCurrentSurface()->clear();
}

GL_APICALL void GL_APIENTRY GL(EnableVertexAttribArray) (GLuint index){
  if (index > NUM_VARYING){
    VR_CONTEXT->setError(GL_INVALID_VALUE);
    return;
  }
  if (index != 0)
  VR_CONTEXT->enableVertexAttribArray(index-1, true);
}

GL_APICALL void GL_APIENTRY GL(VertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr){
  if (indx > NUM_VARYING){
    VR_CONTEXT->setError(GL_INVALID_VALUE);
    return;
  }
  if (indx == 0)
    VR_CONTEXT->setVertices((float*)ptr);
  else
    VR_CONTEXT->setVertexArribPointer(indx-1, (float*)ptr, size);
}

GL_APICALL void GL_APIENTRY GL(DrawArrays) (GLenum mode, GLint first, GLsizei count){
  if (mode == GL_TRIANGLES){
    VR_CONTEXT->drawTriangles(first, count);
  }
}

GL_APICALL GLuint GL_APIENTRY GL(CreateProgram) (void){
  return VR_CONTEXT->makeProgramSlot();
}

GL_APICALL void GL_APIENTRY GL(DeleteShader) (GLuint shader){
  if (shader == 0xffffffff)
    return;
  VR_CONTEXT->setShader(shader, NULL);
}

GL_APICALL GLuint GL_APIENTRY GL(CreateShader) (GLenum type){
  if (type == GL_VERTEX_SHADER)
    return (GLuint)-1;
  return VR_CONTEXT->makeShaderSlot();
}

GL_APICALL void GL_APIENTRY GL(ShaderBinary) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length){
  for (int i = 0; i < n; ++i){
    if ((int)shaders[i] != -1)
      VR_CONTEXT->setShader(shaders[i], (VRShader*)binary);
  }
}

GL_APICALL void GL_APIENTRY GL(AttachShader) (GLuint program, GLuint shader){
  if (shader == 0xffffffff)
    return;
  VR_CONTEXT->addToProgram(program, VR_CONTEXT->getShader(shader));
}

GL_APICALL void GL_APIENTRY GL(LinkProgram) (GLuint program){

}

GL_APICALL void GL_APIENTRY GL(UseProgram) (GLuint program){
  VR_CONTEXT->setProgram(VR_CONTEXT->getProgram(program));
}

GL_APICALL int GL_APIENTRY GL(GetAttribLocation) (GLuint program, const GLchar* name){
  if (strcmp(name, "position") == 0)
    return 0;
  int pos = VR_CONTEXT->getProgram(program)->getVarying(name);
  if (pos < 0)
    return pos;
  return pos+1;
}
