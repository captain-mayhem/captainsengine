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

GL_APICALL void GL_APIENTRY GL(VertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr){
  if (indx == 0)
    VR_CONTEXT->setVertices((float*)ptr);
}

GL_APICALL void GL_APIENTRY GL(DrawArrays) (GLenum mode, GLint first, GLsizei count){
  if (mode == GL_TRIANGLES){
    VR_CONTEXT->drawTriangles(first, count);
  }
}