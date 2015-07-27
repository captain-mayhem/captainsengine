#ifndef ADV_RENDERER_H
#define ADV_RENDERER_H

#include <cstdlib>
#include <system/engine.h>
#include <renderer/GL2/GL2Shader.h>

namespace adv{

class GL1Renderer;
class GL2Renderer;

}

//#define RENDER_TEGRA

#ifdef RENDER_TEGRA
#include <GLES2/gl2.h>
#define RENDER_GL2
#else
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#define RENDER_GL2
#endif

namespace adv{

#ifdef RENDER_GL2

enum MatrixMode{
  MM_MODELVIEW = 0,
  MM_PROJECTION = 1,
  MM_TEXTURE = 2
};
enum AttribType{
  ATTR_VERTEX_ARRAY = 0,
  ATTR_TEXTURE_COORD_ARRAY = 1,
};
#define RND_CLS GL2Renderer
#else

enum MatrixMode{
  MM_MODELVIEW=GL_MODELVIEW,
  MM_PROJECTION=GL_PROJECTION,
  MM_TEXTURE=GL_TEXTURE
};
enum AttribType{
  ATTR_VERTEX_ARRAY = GL_VERTEX_ARRAY,
  ATTR_TEXTURE_COORD_ARRAY = GL_TEXTURE_COORD_ARRAY,
};
#define RND_CLS GL1Renderer
#endif

class AdvRenderer{
public:
  static void init();
  static void deinit();
  static RND_CLS* instance() {return mInstance;}
protected:
  static RND_CLS* mInstance;
};

#ifndef RENDER_GL2

class GL1Renderer : public AdvRenderer{
public:
  void matrixMode(MatrixMode mode){
    glMatrixMode(mode);
  }
  void loadIdentity(){
    glLoadIdentity();
  }
  void translatef(float x, float y, float z){
    glTranslatef(x, y, z);
  }
  void scalef(float x, float y, float z){
    glScalef(x, y, z);
  }
  void rotatef(float angle, float x, float y, float z){
    glRotatf(angle, x, y, z);
  }
  void ortho(float left, float right, float bottom, float top, float neaar, float faar){
    glOrtho(left, right, bottom, top, neaar, faar);
  }
  void pushMatrix(){
    glPushMatrix();
  }
  void popMatrix(){
    glPopMatrix();
  }

  void enable(GLenum cap){
    glEnable(cap);
  }
  void disable(GLenum cap){
    glDisable(cap);
  }
  void enableClientState(GLenum array){
    glEnableClientState(array);
  }

  void vertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer){
    glVertexPointer(size, type, stride, pointer);
  }
  void texCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer){
    glTexCoordPointer(size, type, stride, pointer);
  }

  void color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha){
    glColor4ub(red, green, blue, alpha);
  }

  void drawArrays(GLenum mode, GLint first, GLsizei count){
    glDrawArrays(mode, first, count);
  }
  void drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices){
    glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
  }
};

#else

using CGE::GL2Shader;

}

#include <stack>
#include <math/matrix.h>

namespace adv{

class GL2Renderer : public AdvRenderer{
public:
  GL2Renderer();
  void matrixMode(MatrixMode mode){
    mMatrixMode = mode;
  }
  void loadIdentity(){
    mMatrix[mMatrixMode] = CGE::Matrix(CGE::Matrix::Identity);
  }
  void translatef(float x, float y, float z){
    mMatrix[mMatrixMode] *= CGE::Matrix(CGE::Matrix::Translation, CGE::Vec3f(x,y,z));
  }
  void scalef(float x, float y, float z){
    mMatrix[mMatrixMode] *= CGE::Matrix(CGE::Matrix::Scale, CGE::Vec3f(x,y,z));
  }
  void rotatef(float angle, float x, float y, float z){
    mMatrix[mMatrixMode] *= CGE::Matrix(CGE::Matrix::Rotation, CGE::Vec3f(x,y,z), angle);
  }
  void ortho(float left, float right, float bottom, float top, float neaar, float faar){
    mMatrix[mMatrixMode] *= CGE::Matrix(CGE::Matrix::Ortho, left, right, bottom, top, neaar, faar);
  }
  void pushMatrix(){
    mMatrixStack[mMatrixMode].push(mMatrix[mMatrixMode]);
  }
  void popMatrix(){
    mMatrix[mMatrixMode] = mMatrixStack[mMatrixMode].top();
    mMatrixStack[mMatrixMode].pop();
  }

  void enableClientState(GLenum array){
    glEnableVertexAttribArray(mAttribLocs[array]);
  }
  void enable(GLenum cap){
    if (cap == GL_TEXTURE_2D)
      mShader.uniform(mTextureEnabledLoc, 1);
  }
  void disable(GLenum cap){
    if (cap == GL_TEXTURE_2D)
      mShader.uniform(mTextureEnabledLoc, 0);
  }

  void vertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer){
    glVertexAttribPointer(mAttribLocs[ATTR_VERTEX_ARRAY], size, type, GL_FALSE, stride, pointer);
  }
  void texCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer){
    glVertexAttribPointer(mAttribLocs[ATTR_TEXTURE_COORD_ARRAY], size, type, GL_FALSE, stride, pointer);
  }

  void color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha){
    mShader.uniform(mColorLoc, red/255.0f, green/255.0f, blue/255.0f, alpha/255.0f);
  }

  void drawArrays(GLenum mode, GLint first, GLsizei count){
    applyMatrices();
    glDrawArrays(mode, first, count);
  }
  void drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices){
    applyMatrices();
    glDrawElements(mode, count, type, indices);
  }
protected:
  void applyMatrices(){
    glUniformMatrix4fv(mMatricesLoc, 3, GL_FALSE, mMatrix[0].getData());
  }
  void initShaders();
  std::stack<CGE::Matrix> mMatrixStack[3];
  CGE::Matrix mMatrix[3];
  MatrixMode mMatrixMode;
  GL2Shader mShader;
  int mAttribLocs[2];
  int mMatricesLoc;
  int mTextureLoc;
  int mTextureEnabledLoc;
  int mColorLoc;
};

#endif

}

#define GL() AdvRenderer::instance()->

#endif
