#include "state.h"
#include "triangle.h"
#include "framebuffer.h"
#include "shader.h"

#define GL(a) mygl##a

GL_APICALL void GL_APIENTRY GL(ClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY GL(Clear) (GLbitfield mask);
GL_APICALL void GL_APIENTRY GL(VertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
GL_APICALL void GL_APIENTRY GL(DrawArrays) (GLenum mode, GLint first, GLsizei count);
