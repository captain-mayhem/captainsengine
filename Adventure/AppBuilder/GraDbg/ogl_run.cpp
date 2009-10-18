#include "ogl_run.h"
#include <sstream>
#include <vector>
#include "funcids.h"

#ifdef WIN32
#include <Windows.h>
#define GL_APIENTRY WINAPI
#endif
#include <GL/gl.h>

typedef void (GL_APIENTRY *PFNGLALPHAFUNC)(GLenum func, GLclampf ref);
typedef void (GL_APIENTRY *PFNGLBINDTEXTURE)(GLenum target, GLuint texture);
typedef void (GL_APIENTRY *PFNGLBLENDFUNC)(GLenum sfactor, GLenum dfactor);
typedef void (GL_APIENTRY *PFNGLCLEAR)(GLbitfield mask);
typedef void (GL_APIENTRY *PFNGLCLEARCOLOR)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GL_APIENTRY *PFNGLCOLOR4UB)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (GL_APIENTRY *PFNGLDELETETEXTURES)(GLsizei n, const GLuint *textures);
typedef void (GL_APIENTRY *PFNGLDISABLE)(GLenum cap);
typedef void (GL_APIENTRY *PFNGLDRAWARRAYS)(GLenum mode, GLint first, GLsizei count);
typedef void (GL_APIENTRY *PFNGLENABLE)(GLenum cap);
typedef void (GL_APIENTRY *PFNGLENABLECLIENTSTATE)(GLenum array);
typedef void (GL_APIENTRY *PFNGLGENTEXTURES)(GLsizei n, GLuint *textures);
typedef void (GL_APIENTRY *PFNGLLOADIDENTITY)(void);
typedef void (GL_APIENTRY *PFNGLMATRIXMODE)(GLenum mode);
typedef void (GL_APIENTRY *PFNGLORTHO)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (GL_APIENTRY *PFNGLPOPMATRIX)(void);
typedef void (GL_APIENTRY *PFNGLPUSHMATRIX)(void);
typedef void (GL_APIENTRY *PFNGLSCALEF)(GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRY *PFNGLTEXCOORDPOINTER)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (GL_APIENTRY *PFNGLTEXIMAGE2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRY *PFNGLTEXPARAMETERI)(GLenum target, GLenum pname, GLint param);
typedef void (GL_APIENTRY *PFNGLTEXSUBIMAGE2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GL_APIENTRY *PFNGLTRANSLATEF)(GLfloat x, GLfloat y, GLfloat z);
typedef void (GL_APIENTRY *PFNGLVERTEXPOINTER)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

PFNGLALPHAFUNC alphaFunc = NULL;
PFNGLBINDTEXTURE bindTexture = NULL;
PFNGLBLENDFUNC blendFunc = NULL;
PFNGLCLEAR clear = NULL;
PFNGLCLEARCOLOR clearColor = NULL;
PFNGLCOLOR4UB color4ub = NULL;
PFNGLDELETETEXTURES deleteTextures = NULL;
PFNGLDISABLE disable = NULL;
PFNGLDRAWARRAYS drawArrays = NULL;
PFNGLENABLE enable = NULL;
PFNGLENABLECLIENTSTATE enableClientState = NULL;
PFNGLGENTEXTURES genTextures = NULL;
PFNGLLOADIDENTITY loadIdentity = NULL;
PFNGLMATRIXMODE matrixMode = NULL;
PFNGLORTHO ortho = NULL;
PFNGLPOPMATRIX popMatrix = NULL;
PFNGLPUSHMATRIX pushMatrix = NULL;
PFNGLSCALEF scalef = NULL;
PFNGLTEXCOORDPOINTER texCoordPointer = NULL;
PFNGLTEXIMAGE2D texImage2D = NULL;
PFNGLTEXPARAMETERI texParameteri = NULL;
PFNGLTEXSUBIMAGE2D texSubImage2D = NULL;
PFNGLTRANSLATEF translatef = NULL;
PFNGLVERTEXPOINTER vertexPointer = NULL;

static void cleanup();

struct OGLRunData{
  OGLRunData() : mIsReplaying(false){
    mTexDatas.push_back(NULL);
    atexit(cleanup);
  }
  bool mIsReplaying;
  std::vector<GLuint> mTextures;
  std::vector<const GLvoid*> mTexDatas;
  std::vector<const GLvoid*> mVertexDatas;
  std::vector<const GLvoid*> mTexCoordDatas;
};

static OGLRunData* gData = NULL;

void cleanup(){
  delete gData;
}

void gl_init(void* lib){
  gData = new OGLRunData();
  alphaFunc = (PFNGLALPHAFUNC)getSymbol(lib, "glAlphaFunc");
  bindTexture = (PFNGLBINDTEXTURE)getSymbol(lib, "glBindTexture");
  blendFunc = (PFNGLBLENDFUNC)getSymbol(lib, "glBlendFunc");
  clear = (PFNGLCLEAR)getSymbol(lib, "glClear");
  clearColor = (PFNGLCLEARCOLOR)getSymbol(lib, "glClearColor");
  color4ub = (PFNGLCOLOR4UB)getSymbol(lib, "glColor4ub");
  deleteTextures = (PFNGLDELETETEXTURES)getSymbol(lib, "glDeleteTextures");
  disable = (PFNGLDISABLE)getSymbol(lib, "glDisable");
  drawArrays = (PFNGLDRAWARRAYS)getSymbol(lib, "glDrawArrays");
  enable = (PFNGLENABLE)getSymbol(lib, "glEnable");
  enableClientState = (PFNGLENABLECLIENTSTATE)getSymbol(lib, "glEnableClientState");
  genTextures = (PFNGLGENTEXTURES)getSymbol(lib, "glGenTextures");
  loadIdentity = (PFNGLLOADIDENTITY)getSymbol(lib, "glLoadIdentity");
  matrixMode = (PFNGLMATRIXMODE)getSymbol(lib, "glMatrixMode");
  ortho = (PFNGLORTHO)getSymbol(lib, "glOrtho");
  popMatrix = (PFNGLPOPMATRIX)getSymbol(lib, "glPopMatrix");
  pushMatrix = (PFNGLPUSHMATRIX)getSymbol(lib, "glPushMatrix");
  scalef = (PFNGLSCALEF)getSymbol(lib, "glScalef");
  texCoordPointer = (PFNGLTEXCOORDPOINTER)getSymbol(lib, "glTexCoordPointer");
  texImage2D = (PFNGLTEXIMAGE2D)getSymbol(lib, "glTexImage2D");
  texParameteri = (PFNGLTEXPARAMETERI)getSymbol(lib, "glTexParameteri");
  texSubImage2D = (PFNGLTEXSUBIMAGE2D)getSymbol(lib, "glTexSubImage2D");
  translatef = (PFNGLTRANSLATEF)getSymbol(lib, "glTranslatef");
  vertexPointer = (PFNGLVERTEXPOINTER)getSymbol(lib, "glVertexPointer");
}

void* gl_interpret(const std::string& cmd){
  int opcode;
  std::istringstream in(cmd);
  in >> opcode;
  switch(opcode){
    case GLALPHAFUNC:
      {
        GLenum func; in >> func;
        GLclampf ref; in >> ref;
        alphaFunc(func, ref);
        break;
      }
    case GLBINDTEXTURE:
      {
        GLenum target; in >> target;
        int tex; in >> tex;
        bindTexture(target, gData->mTextures[tex]);
        break;
      }
    case GLBLENDFUNC:
      {
        GLenum sfactor, dfactor;
        in >> sfactor >> dfactor;
        blendFunc(sfactor, dfactor);
        break;
      }
    case GLCLEAR:
      {
        GLbitfield mask; in >> mask;
        clear(mask);
        break;
      }
    case GLCLEARCOLOR:
      {
        GLclampf red, green, blue, alpha;
        in >> red >> green >> blue >> alpha;
        clearColor(red, green, blue, alpha);
        break;
      }
    case GLCOLOR4UB:
      {
        GLubyte red, green, blue, alpha;
        in >> red >> green >> blue >> alpha;
        color4ub(red, green, blue, alpha);
        break;
      }
    case GLDRAWARRAYS:
      {
        GLenum mode; in >> mode;
        GLint first; in >> first;
        GLsizei count; in >> count;
        drawArrays(mode, first, count);
        break;
      }
    case GLDELETETEXTURES:
      {
        GLsizei n; in >> n;
        GLuint idx; in >> idx;
        deleteTextures(n, &gData->mTextures[idx]);
        break;
      }
    case GLDISABLE:
      {
        GLenum cap; in >> cap;
        disable(cap);
        break;
      }
    case GLENABLE:
      {
        GLenum cap; in >> cap;
        enable(cap);
        break;
      }
    case GLENABLECLIENTSTATE:
      {
        GLenum array; in >> array;
        enableClientState(array);
        break;
      }
    case GLGENTEXTURES:
      {
        GLsizei n; in >> n;
        void* tmp; in >> tmp; GLuint* textures = (GLuint*)tmp;
        if (gData->mIsReplaying)
          textures = new GLuint[n];
        genTextures(n, textures);
        for (GLsizei i = 0; i < n; ++i){
          gData->mTextures.push_back(textures[i]);
        }
        if (gData->mIsReplaying)
          delete [] textures;
        break;
      }
    case GLLOADIDENTITY:
      {
        loadIdentity();
        break;
      }
    case GLMATRIXMODE:
      {
        GLenum mode; in >> mode;
        matrixMode(mode);
        break;
      }
    case GLORTHO:
      {
        GLdouble left, right, bottom, top, zNear, zFar;
        in >> left; in >> right; in >> bottom; in >> top; in >> zNear; in >> zFar;
        ortho(left, right, bottom, top, zNear, zFar);
        break;
      }
    case GLPOPMATRIX:
      {
        popMatrix();
        break;
      }
    case GLPUSHMATRIX:
      {
        pushMatrix();
        break;
      }
    case GLSCALEF:
      {
        GLfloat x, y, z;
        in >> x >> y >> z;
        scalef(x, y, z);
        break;
      }
    case GLTEXCOORDPOINTER:
      {
        GLint size; in >> size;
        GLenum type; in >> type;
        GLsizei stride; in >> stride;
        int pointer; in >> pointer;
        texCoordPointer(size, type, stride, gData->mTexCoordDatas[pointer]);
        break;
      }
    case GLTEXIMAGE2D:
      {
        GLenum target; in >> target;
        GLint level; in >> level;
        GLint internalformat; in >> internalformat;
        GLsizei width; in >> width;
        GLsizei height; in >> height;
        GLint border; in >> border;
        GLenum format; in >> format;
        GLenum type; in >> type;
        int pixels; in >> pixels;
        texImage2D(target, level, internalformat, width, height, border, format, type, gData->mTexDatas[pixels]);
        break;
      }
    case GLTEXPARAMETERI:
      {
        GLenum target; in >> target;
        GLenum pname; in >> pname;
        GLint param; in >> param;
        texParameteri(target, pname, param);
        break;
      }
    case GLTEXSUBIMAGE2D:
      {
        GLenum target; in >> target;
        GLint level; in >> level;
        GLint xoffset; in >> xoffset;
        GLint yoffset; in >> yoffset;
        GLsizei width; in >> width;
        GLsizei height; in >> height;
        GLenum format; in >> format;
        GLenum type; in >> type;
        int pixels; in >> pixels;
        texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, gData->mTexDatas[pixels]);
        break;
      }
    case GLTRANSLATEF:
      {
        GLfloat x, y, z;
        in >> x >> y >> z;
        translatef(x, y, z);
        break;
      }
    case GLVERTEXPOINTER:
      {
        GLint size; in >> size;
        GLenum type; in >> type;
        GLsizei stride; in >> stride;
        int pointer; in >> pointer;
        vertexPointer(size, type, stride, gData->mVertexDatas[pointer]);
        break;
      }
  }
  return NULL;
}

IMPEXP void gl_add_texturedata(const void* data){
  gData->mTexDatas.push_back(data);
}

IMPEXP void gl_add_vertexdata(const void* data){
  gData->mVertexDatas.push_back(data);
}

IMPEXP void gl_add_texcoorddata(const void* data){
  gData->mTexCoordDatas.push_back(data);
}
