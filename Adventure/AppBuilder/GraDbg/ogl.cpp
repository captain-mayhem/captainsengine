#ifdef WIN32
#define WINGDIAPI __declspec(dllexport)
#define APIENTRY
#define GL_API WINGDIAPI
#define GL_APIENTRY APIENTRY
#endif

extern "C"{
#include <GL/gl.h>
};

GL_API void GL_APIENTRY glAlphaFunc (GLenum func, GLclampf ref){

}

GL_API void GL_APIENTRY glBegin (GLenum mode){

}

GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture){

}

GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor){

}

GL_API void GL_APIENTRY glClear (GLbitfield mask){

}

GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){

}

GL_API void GL_APIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue){

}

GL_API void GL_APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha){

}

GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures){

}

GL_API void GL_APIENTRY glDisable (GLenum cap){

}

GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count){

}

GL_API void GL_APIENTRY glEnable (GLenum cap){

}

GL_API void GL_APIENTRY glEnableClientState (GLenum array){

}

GL_API void GL_APIENTRY glEnd (void){

}

GL_API void GL_APIENTRY glEvalCoord1f (GLfloat u){

}

GL_API void GL_APIENTRY glEvalCoord2f (GLfloat u, GLfloat v){

}

GL_API void GL_APIENTRY glEvalMesh1 (GLenum mode, GLint i1, GLint i2){

}

GL_API void GL_APIENTRY glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2){

}

GL_API void GL_APIENTRY glEvalPoint2 (GLint i, GLint j){

}

GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures){

}

GL_API GLenum GL_APIENTRY glGetError (void){
  return GL_NONE;
}

GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *params){

}

GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *params){

}

GL_API const GLubyte * GL_APIENTRY glGetString (GLenum name){
  return (const GLubyte*)"";
}

GL_API void GL_APIENTRY glLoadIdentity (void){

}

GL_API void GL_APIENTRY glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points){

}

GL_API void GL_APIENTRY glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points){

}

GL_API void GL_APIENTRY glMapGrid1f (GLint un, GLfloat u1, GLfloat u2){

}

GL_API void GL_APIENTRY glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2){

}

GL_API void GL_APIENTRY glMatrixMode (GLenum mode){

}

GL_API void GL_APIENTRY glMultMatrixd (const GLdouble *m){

}

GL_API void GL_APIENTRY glMultMatrixf (const GLfloat *m){

}

GL_API void GL_APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz){

}

GL_API void GL_APIENTRY glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar){

}

GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param){

}

GL_API void GL_APIENTRY glPolygonMode (GLenum face, GLenum mode){

}

GL_API void GL_APIENTRY glPopAttrib (void){

}

GL_API void GL_APIENTRY glPopMatrix (void){

}

GL_API void GL_APIENTRY glPushAttrib (GLbitfield mask){

}

GL_API void GL_APIENTRY glPushMatrix (void){

}

GL_API void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z){

}

GL_API void GL_APIENTRY glTexCoord2f (GLfloat s, GLfloat t){

}

GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){

}

GL_API void GL_APIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels){

}

GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels){

}

GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param){

}

GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels){

}

GL_API void GL_APIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z){

}

GL_API void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z){

}

GL_API void GL_APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z){

}

GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){

}

GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height){

}
