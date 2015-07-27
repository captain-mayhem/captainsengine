#include <sstream>
#include <map>
#include "funcids.h"
#include "ogl_run.h"

#ifdef WIN32
#define APIENTRY __stdcall
#define WINGDIAPI __declspec(dllexport)
#define GL_API WINGDIAPI
#define GL_APIENTRY APIENTRY
#endif

//extern "C"{
#ifdef USE_GLES
#include <GLES/gl.h>
#ifndef GL_NONE
#define GL_NONE 0
#endif
#ifndef GLdouble
#define GLdouble double
#endif
#else
#include <GL/gl.h>
#ifndef GLfixed
#define GLfixed int
#endif
#endif
//};

//#undef GL_APIENTRY
#undef GL_API
//#define GL_APIENTRY
#define GL_API

const char* SEP = " ";

static void cleanup();

struct OGLData{
  OGLData() : mTexcount(-1), mTexDatCount(0), mVertDatCount(-1), mTexCoordDatCount(-1){
    atexit(cleanup);
  }
  int mTexcount;
  std::map<GLuint, int> mTextures;
  int mTexDatCount;
  int mVertDatCount;
  int mTexCoordDatCount;
};

static OGLData* gData = NULL;

void cleanup(){
  delete gData;
}

void ogl_init(){
  gData = new OGLData();
}

GL_API void GL_APIENTRY glAlphaFunc (GLenum func, GLclampf ref){
  std::ostringstream out;
  out << GLALPHAFUNC << SEP << func << SEP << ref;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glBegin (GLenum mode){
  std::ostringstream out;
  out << GLBEGIN << SEP << mode;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture){
  std::ostringstream out;
  out << GLBINDTEXTURE << SEP << target << SEP << gData->mTextures[texture];
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor){
  std::ostringstream out;
  out << GLBLENDFUNC << SEP << sfactor << SEP << dfactor;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glClear (GLbitfield mask){
  std::ostringstream out;
  out << GLCLEAR << SEP << mask;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){
  std::ostringstream out;
  out << GLCLEARCOLOR << SEP << red << SEP << green << SEP << blue << SEP << alpha;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glClearStencil (GLint s){
  std::ostringstream out;
  out << GLCLEARSTENCIL << SEP << s;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue){
  std::ostringstream out;
  out << GLCOLOR3F << SEP << red << SEP << green << SEP << blue;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){
  std::ostringstream out;
  out << GLCOLOR4F << SEP << red << SEP << green << SEP << blue << SEP << alpha;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha){
  std::ostringstream out;
  out << GLCOLOR4UB << SEP << red << SEP << green << SEP << blue << SEP << alpha;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha){

}

GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures){
  std::ostringstream out;
  out << GLDELETETEXTURES << SEP << n << SEP << gData->mTextures[textures[0]];
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glDisable (GLenum cap){
  std::ostringstream out;
  out << GLDISABLE << SEP << cap;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count){
  std::ostringstream out;
  out << GLDRAWARRAYS << SEP << mode << SEP << first << SEP << count;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glEnable (GLenum cap){
  std::ostringstream out;
  out << GLENABLE << SEP << cap;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glEnableClientState (GLenum array){
  std::ostringstream out;
  out << GLENABLECLIENTSTATE << SEP << array;
  gl_interpret(out.str());
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
  std::ostringstream out;
  out << GLGENTEXTURES << SEP << n << SEP << textures;
  gl_interpret(out.str());
  for (GLsizei i = 0; i < n; ++i){
    gData->mTextures[textures[i]] = ++gData->mTexcount;
  }
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

GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode){

}

GL_API void GL_APIENTRY glLoadIdentity (void){
  std::ostringstream out;
  out << GLLOADIDENTITY;
  gl_interpret(out.str());
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
  std::ostringstream out;
  out << GLMATRIXMODE << " " << mode;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glMultMatrixd (const GLdouble *m){

}

GL_API void GL_APIENTRY glMultMatrixf (const GLfloat *m){

}

GL_API void GL_APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz){

}

GL_API void GL_APIENTRY glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar){
  std::ostringstream out;
  out << GLORTHO << " " << left << " " << right << " " << bottom << " " << top << " " << zNear << " " << zFar;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar){

}

GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param){

}

GL_API void GL_APIENTRY glPolygonMode (GLenum face, GLenum mode){

}

GL_API void GL_APIENTRY glPopAttrib (void){

}

GL_API void GL_APIENTRY glPopMatrix (void){
  std::ostringstream out;
  out << GLPOPMATRIX;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glPushAttrib (GLbitfield mask){

}

GL_API void GL_APIENTRY glPushMatrix (void){
  std::ostringstream out;
  out << GLPUSHMATRIX;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z){
  std::ostringstream out;
  out << GLSCALEF << SEP << x << SEP << y << SEP << z;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glShadeModel (GLenum mode){

}

GL_API void GL_APIENTRY glTexCoord2f (GLfloat s, GLfloat t){

}

GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){
  std::ostringstream out;
  int texcoorddatid = ++gData->mTexCoordDatCount;
  gl_add_texcoorddata(pointer);
  out << GLTEXCOORDPOINTER << SEP << size << SEP << type << SEP << stride << SEP << texcoorddatid;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glTexEnvx (GLenum target, GLenum pname, GLfixed param){

}

GL_API void GL_APIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels){

}

GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels){
  std::ostringstream out;
  int texdatid = 0;
  if (pixels != NULL){
    texdatid = ++gData->mTexDatCount;
    gl_add_texturedata(pixels);
  }
  out << GLTEXIMAGE2D << SEP << target << SEP << level << SEP << internalformat << SEP << width << SEP << height << SEP << border << SEP << format << SEP << type << SEP << texdatid;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param){
  std::ostringstream out;
  out << GLTEXPARAMETERI << SEP << target << SEP << pname << SEP << param;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param){

}

GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels){
  std::ostringstream out;
  int texdatid = ++gData->mTexDatCount;
  gl_add_texturedata(pixels);
  out << GLTEXSUBIMAGE2D << SEP << target << SEP << level << SEP << xoffset << SEP << yoffset << SEP << width << SEP << height << SEP << format << SEP << type << SEP << texdatid;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z){

}

GL_API void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z){
  std::ostringstream out;
  out << GLTRANSLATEF << SEP << x << SEP << y << SEP << z;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z){

}

GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){
  std::ostringstream out;
  int vertdatid = ++gData->mVertDatCount;
  gl_add_vertexdata(pointer);
  out << GLVERTEXPOINTER << SEP << size << SEP << type << SEP << stride << SEP << vertdatid;
  gl_interpret(out.str());
}

GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height){

}
