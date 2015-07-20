#include "state.h"
#include "triangle.h"
#include "framebuffer.h"
#include "shader.h"

#define GL_APICALL
#ifdef WIN32
#define GL_APIENTRY __stdcall
#else
#define GL_APIENTRY
#endif

typedef void GLvoid;
typedef char GLchar;
typedef float GLfloat;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef int GLsizei;

#define GL_COLOR_BUFFER_BIT                              0x00004000

#define GL_FALSE                                         0

#define GL_TRIANGLES                                     0x0004

#define GL_INVALID_VALUE                                 0x0501

#define GL_FLOAT                                         0x1406

#define GL_FRAGMENT_SHADER                               0x8B30
#define GL_VERTEX_SHADER                                 0x8B31

#define GL(a) vrgl##a

#define VR_SHADER_BINARY_FORMAT 0x666

GL_APICALL void GL_APIENTRY GL(ClearColor) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY GL(Clear) (GLbitfield mask);
GL_APICALL void GL_APIENTRY GL(EnableVertexAttribArray) (GLuint index);
GL_APICALL void GL_APIENTRY GL(VertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
GL_APICALL void GL_APIENTRY GL(DrawArrays) (GLenum mode, GLint first, GLsizei count);

GL_APICALL GLuint GL_APIENTRY GL(CreateProgram) (void);
GL_APICALL GLuint GL_APIENTRY GL(CreateShader) (GLenum type);
GL_APICALL void GL_APIENTRY GL(DeleteShader) (GLuint shader);
GL_APICALL void GL_APIENTRY GL(ShaderBinary) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
GL_APICALL void GL_APIENTRY GL(AttachShader) (GLuint program, GLuint shader);
GL_APICALL void GL_APIENTRY GL(LinkProgram) (GLuint program);
GL_APICALL void GL_APIENTRY GL(UseProgram) (GLuint program);
GL_APICALL int GL_APIENTRY GL(GetAttribLocation) (GLuint program, const GLchar* name);