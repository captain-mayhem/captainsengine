#ifndef ADV_RENDERER_H
#define ADV_RENDERER_H

#include <cstdlib>
#include <system/engine.h>

class GL1Renderer;
class GL2Renderer;

//#define RENDER_TEGRA

#ifdef RENDER_TEGRA
//#include <GLES2/gl.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>

enum MatrixMode{
    MM_MODELVIEW = 0,
    MM_PROJECTION = 1,
    MM_TEXTURE = 2
};
#define RND_CLS GL2Renderer
#else
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>

enum MatrixMode{
  MM_MODELVIEW=GL_MODELVIEW,
  MM_PROJECTION=GL_PROJECTION,
  MM_TEXTURE=GL_TEXTURE
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

#ifndef RENDER_TEGRA

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
  void ortho(float left, float right, float bottom, float top, float neaar, float faar){
    glOrtho(left, right, bottom, top, neaar, faar);
  }
  void pushMatrix(){
    glPushMatrix();
  }
  void popMatrix(){
    glPopMatrix();
  }
  void drawArrays(GLenum mode, GLint first, GLsizei count){
    glDrawArrays(mode, first, count);
  }
  void drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices){
    glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
  }
};

#else

#include <stack>
#include <math/matrix.h>

class GL2Renderer : public AdvRenderer{
public:
  GL2Renderer() : mMatrixMode(MM_MODELVIEW){
    for (int i = 0; i < 3; ++i){
      mMatrix[i] = CGE::Matrix(CGE::Matrix::Identity);
    }
  }
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
  void drawArrays(GLenum mode, GLint first, GLsizei count){
    applyMatrices();
    glDrawArrays(mode, first, count);
  }
  void drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices){
    applyMatrices();
    glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
  }
protected:
  void applyMatrices(){
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mMatrix[0].getData());
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(mMatrix[1].getData());
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(mMatrix[2].getData());
  }
  std::stack<CGE::Matrix> mMatrixStack[3];
  CGE::Matrix mMatrix[3];
  MatrixMode mMatrixMode;
};

#endif

#define GL() AdvRenderer::instance()->

#endif
