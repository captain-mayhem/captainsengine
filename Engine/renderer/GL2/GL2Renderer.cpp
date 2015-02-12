#define _USE_MATH_DEFINES
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "../../window/nativeWindows.h"
#include "../../window/nativeLinux.h"
#include "../../system/engine.h"
#ifdef QNX
#include <GLES2/gl2.h>
#define glClearDepth glClearDepthf
#else
#include <GL/glew.h>
#endif
//#include <GL/gl.h>
#include "GL2Renderer.h"
#include "GL2vertexbuffer.h"
#include "../OpenGL/OGLtexture.h"
#include "GL2indexbuffer.h"
#include "GL2Shader.h"
#include "GL2rendertarget.h"

using namespace CGE;

TR_CHANNEL(CGE_Renderer_GL2);

GL2Renderer::GL2Renderer(): Renderer(), mShader(NULL) {
  type_ = OpenGL2;
#ifdef WIN32
  hDC_ = NULL;
  hRC_ = NULL;
#endif
#if defined UNIX && !defined QNX
  glx_ = NULL;
#endif
  for (int i = 0; i < 3; ++i){
    mMatrix[i] = CGE::Matrix(CGE::Matrix::Identity);
  }
  mMatrixMode = Modelview;
}

GL2Renderer::~GL2Renderer(){
#if defined(WIN32)
  wglDeleteContext(hRC_);
#endif
}

void GL2Renderer::initContext(AppWindow* win){
  TR_USE(CGE_Renderer_GL2);
  TR_INFO("Initializing OpenGL context");
  win_ = win;
#if defined(WIN32) && !defined(UNDER_CE)
  static PIXELFORMATDESCRIPTOR pfd ={
    sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,
      win->getBpp(),
      0, 0, 0, 0, 0, 0,   //Color bits ignored
      0,					//No Alpha buffer
      0,					//shift bit ignored
      0,					//no accumulation buffer
      0, 0, 0, 0,			//accumulation bits ignored
      16,					//Z-buffer depth
      0,					//no stencil buffer
      0,					//no auxiliary buffer
      PFD_MAIN_PLANE,
      0,					//Reserved
      0, 0, 0				//Layer masks ignored
  };

  HWND wnd = (HWND)static_cast<WindowsWindow*>(win)->getHandle();
  if(!(hDC_ = GetDC(wnd))){
    TR_ERROR("Can't create GL device context");
    EXIT();
  }

  GLuint pixelFormat;
  if(!(pixelFormat = ChoosePixelFormat(hDC_, &pfd))){
    TR_ERROR("Can't find a suitable PixelFormat");
    EXIT();
  }

  if (!SetPixelFormat(hDC_, pixelFormat, &pfd)){
    TR_ERROR("Can't set the PixelFormat");
    EXIT();
  }

  if (!(hRC_ = wglCreateContext(hDC_))){
    TR_ERROR("Can't create GL rendering context");
    EXIT();
  }

  if (!wglMakeCurrent(hDC_, hRC_)){
    TR_ERROR("Cant't activate GL rendering context");
    EXIT();
  }

#endif
#if defined UNIX && !defined QNX
  X11Window* x11 = static_cast<X11Window* >(win_);
  glXMakeCurrent(x11->getDisplay(), x11->getWindow(), glx_);
#endif
#ifdef WIN32
  resizeScene(win->getWidth(), win->getHeight());
#endif
  //initRendering();  
  
}

void GL2Renderer::killContext(){
  TR_USE(CGE_Renderer_GL2);
  delete mRT;
  delete mShader;
  delete mLightShader;
#if defined(WIN32) && !defined(UNDER_CE)
  if (hRC_){
    if (!wglMakeCurrent(NULL,NULL)){
      TR_ERROR("Release of GL context failed");
    }
    if (!wglDeleteContext(hRC_)){
      TR_ERROR("Release of rendering context failed");
    }
    hRC_ = NULL;
  }

  HWND wnd = (HWND)static_cast<WindowsWindow*>(win_)->getHandle();
  if (hDC_ && !ReleaseDC(wnd,hDC_)){
    TR_ERROR("Release of device context failed");
    hDC_ = NULL;
  }
#endif
#if defined UNIX && !defined QNX
  X11Window* x11 = static_cast<X11Window* >(win_);
  if (glx_){
    if (!glXMakeCurrent(x11->getDisplay(), None, NULL)){
      TR_ERROR("Release of GL context failed");
    }
    glXDestroyContext(x11->getDisplay(), glx_);
    glx_ = NULL;
  }
#endif
}

static char const * vs_src_unlit =
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 texmat;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec4 vcolor;\n"
"\n"
"void main(){\n"
"  uvcoord = (texmat*vec4(texcoord,1,1)).xy;\n"
"  vcolor = color;\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"}\n"
"";

static char const * fs_src_unlit =
"uniform sampler2D texture;\n"
"uniform bool textureEnabled;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec4 vcolor;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  if (textureEnabled)\n"
"     color = texture2D(texture, uvcoord);\n"
"  gl_FragColor = color*vcolor;\n"
"}\n"
"";

namespace CGE{
  class LightGL2Shader : public GL2Shader{
    virtual bool linkShaders(){
      bool ret = GL2Shader::linkShaders();
      mTexEnabledLoc = getUniformLocation(FRAGMENT_SHADER, "textureEnabled");
      mColorLoc = getUniformLocation(FRAGMENT_SHADER, "matDiffuse");
      mAmbientLoc = getUniformLocation(FRAGMENT_SHADER, "matAmbient");
      mShininessLoc = getUniformLocation(FRAGMENT_SHADER, "matShininess");
      mSpecularLoc = getUniformLocation(FRAGMENT_SHADER, "matSpecular");
      mLightPosLoc = getUniformLocation(FRAGMENT_SHADER, "lightPos");
      return ret;
    }
    virtual bool applyMaterial(Material const& mat){
      lockUniforms(FRAGMENT_SHADER);
      Color diff = mat.getDiffuse();
      diff.a = mat.getOpacity();
      uniform(mColorLoc, diff);
      uniform(mAmbientLoc, mat.getAmbient());
      uniform(mShininessLoc, mat.getPower());
      uniform(mSpecularLoc, mat.getSpecular());
      Texture const* tex = mat.getDiffuseTex();
      if (!tex)
        tex = mat.getOpacityTex();
      if (tex){
        uniform(mTexEnabledLoc, 1);
        tex->activate();
      }
      else
        uniform(mTexEnabledLoc, 0);
      unlockUniforms(FRAGMENT_SHADER);
      return true;
    }
    virtual void applyLight(int number, Light const& light){
      lockUniforms(FRAGMENT_SHADER);
      Vec4f pos = light.getPosition();
      uniform(mLightPosLoc, pos.x, pos.y, pos.z, pos.w);
      unlockUniforms(FRAGMENT_SHADER);
    }

    int mColorLoc;
    int mAmbientLoc;
    int mSpecularLoc;
    int mTexEnabledLoc;
    int mShininessLoc;
    
    int mLightPosLoc;
  };
}

static char const * vs_src_light =
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 texmat;\n"
"uniform mat4 mvmat;\n"
"uniform mat4 normalmat;\n"
"\n"
"varying vec2 uvcoord;\n"
//"varying vec4 vcolor;\n"

"varying vec3 vnormal;\n"
//"varying vec3 lightvec;\n"
//"varying vec3 lightpos;\n"
"varying vec3 vpos;\n"
"\n"
"void main(){\n"
"  uvcoord = (texmat*vec4(texcoord,1,1)).xy;\n"
//"  vcolor = color;\n"
"\n"
"  vnormal = normalize((normalmat * vec4(normal, 0.0)).xyz);\n"
"  vpos = (mvmat*vec4(pos, 1.0)).xyz;\n"
//"  vec3 lpos =  (0,1.0,0.0);\n"
//"  lightpos = lpos;\n"
//"  vec3 lpos = (mvmat*vec4(0.0, 3.0, 3.0, 1.0)).xyz;\n"
//"  lightvec = normalize(lpos - vpos);\n"
"\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"}\n"
"";

static char const * fs_src_light =
"uniform sampler2D texture;\n"
"uniform bool textureEnabled;\n"
"\n"
"varying vec2 uvcoord;\n"
//"varying vec4 vcolor;\n"
"\n"
"varying vec3 vnormal;\n"
"varying vec3 vpos;\n"
"\n"
"uniform vec4 lightPos;\n"
"\n"
"uniform vec4 matAmbient;\n"
"uniform vec4 matDiffuse;\n"
"uniform vec4 matSpecular;\n"
"uniform float matShininess;\n"
"\n"
"void main(){\n"
"  vec4 color = matDiffuse;\n"
"  if (textureEnabled)\n"
"     color *= texture2D(texture, uvcoord);\n"
"\n"
"  vec3 lightvec;\n"
"  float att = 1.0f;\n"
"  if (lightPos.w == 0.0)\n"
"     lightvec = normalize(lightPos.xyz);\n"
"  else{\n"
"    lightvec = normalize(lightPos.xyz - vpos);\n"
"    float lightDist = length(lightPos.xyz-vpos);\n"
"    att = 1.0/(1.0+0.1*pow(lightDist, 2));\n"
"  }\n"
"  vec3 normal = normalize(vnormal);\n"
"  vec3 eye = normalize(-vpos);\n"
"  vec3 refl = normalize(reflect(-lightvec, normal));\n"
"  float NL = max(dot(normal,lightvec), 0.0);\n"
"  float spec = 0.0;\n"
"  if (NL > 0.0)\n"
"    spec = pow(max(dot(refl, eye), 0.0), matShininess);\n"
"\n"
"  vec3 ambient = matAmbient.rgb;\n"
"  vec3 diffuse = vec3(1.0,1.0,1.0)*NL;\n"
"  vec3 specular = vec3(1.0,1.0, 1.0)*spec;\n"
"  vec4 finalColor = vec4(color.rgb*(ambient + diffuse*att) + specular*att*matSpecular.rgb, color.a);\n"
"  gl_FragColor = finalColor;\n"
//"  gl_FragColor = vec4(1,1,0,1);\n"
//"  gl_FragColor = vec4(spec, spec, spec, 1.0);\n"
"}\n"
"";

void GL2Renderer::initRendering(){
  TR_USE(CGE_Renderer_GL2);
  TR_INFO("Initializing Scene");
#ifndef QNX
  GLenum err = glewInit();
  if (err != GLEW_OK){
    TR_ERROR("Unable to init OpenGL extensions");
    CGE::Engine::instance()->requestShutdown();
    return;
  }
  if (!GLEW_VERSION_2_0){
    TR_ERROR("OpenGL 2.0 not available");
    CGE::Engine::instance()->requestShutdown();
    return;
  }
#endif
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //depth buffer
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  mRT = new GL2RenderTarget();
  RenderTarget::mCurrTarget = mRT;

  mLightShader = new CGE::LightGL2Shader();
  mLightShader->addShader(Shader::VERTEX_SHADER, vs_src_light);
  mLightShader->addShader(Shader::FRAGMENT_SHADER, fs_src_light);
  mLightShader->bindAttribLocation(0, "pos");
  mLightShader->bindAttribLocation(1, "color");
  mLightShader->bindAttribLocation(2, "texcoord");
  mLightShader->bindAttribLocation(3, "normal");
  mLightShader->linkShaders();
  mLightShader->syncMatrix("mvp", CGE::MVP);
  mLightShader->syncMatrix("texmat", CGE::MatTexture);
  mLightShader->syncMatrix("mvmat", CGE::Modelview);
  mLightShader->syncMatrix("normalmat", CGE::MatNormal);
  mLightShader->activate();
  int tex = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "texture");
  mLightShader->uniform(tex, 0);//texture (uniform 32) at stage 0
  mLightShader->deactivate();

  mShader = new CGE::GL2Shader();
  mShader->addShader(Shader::VERTEX_SHADER, vs_src_unlit);
  mShader->addShader(Shader::FRAGMENT_SHADER, fs_src_unlit);
  mShader->bindAttribLocation(0, "pos");
  mShader->bindAttribLocation(1, "color");
  mShader->bindAttribLocation(2, "texcoord");
  mShader->bindAttribLocation(3, "normal");
  mShader->linkShaders();
  mShader->syncMatrix("mvp", CGE::MVP);
  mShader->syncMatrix("texmat", CGE::MatTexture);
  mShader->activate();
  tex = mShader->getUniformLocation(Shader::FRAGMENT_SHADER, "texture");
  mShader->uniform(tex, 0);//texture (uniform 32) at stage 0

  Renderer::initRendering();
}

void GL2Renderer::renderScene(){
  Renderer::renderScene();
}

void GL2Renderer::resizeScene(int width, int height){
  TR_USE(CGE_Renderer_GL2);
#ifdef WIN32
  if (hRC_ == NULL)
    return;
#endif
  TR_INFO("Resizing Scene");
  if (height == 0){
    height = 1;
  }
  win_->setWidth(width);
  win_->setHeight(height);

  glViewport(0, 0, width, height);

  Renderer::resizeScene(width, height);
}

//! clear scene
void GL2Renderer::setClearColor(::CGE::Vector3D color){
  glClearColor(color.x, color.y, color.z, 1.0);
}

void GL2Renderer::clear(long flags){
  long glflags = 0;
  if (flags & ZBUFFER)
    glflags |= GL_DEPTH_BUFFER_BIT;
  if (flags & COLORBUFFER)
    glflags |= GL_COLOR_BUFFER_BIT;
  glClear(glflags);
}

//! get a vertex buffer
VertexBuffer* GL2Renderer::createVertexBuffer(){
  return new GL2VertexBuffer();
}

Texture* GL2Renderer::createTexture(Image* img, Texture::Format fmt){
  OGLTexture* tex = new OGLTexture();
  if (tex->createFromImage(img, fmt))
    return tex;
  delete tex;
  return NULL;
}

IndexBuffer* GL2Renderer::createIndexBuffer(IndexBuffer::Type t, uint32 size){
  return new GL2IndexBuffer(t,size);
}

RenderTarget* GL2Renderer::createRenderTarget(unsigned width, unsigned height){
  GL2RenderTarget* rt = new GL2RenderTarget(width, height);
  return rt;
}

void GL2Renderer::lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up){
  Vec3f forward = look - position;
  forward.normalize();
  Vec3f side = forward.cross(up).normalized();
  Vec3f up_new = side.cross(forward);
  Matrix mat = Matrix(side, up_new, forward*-1, Vec3f()/*eye*1*/)*Matrix(Matrix::Translation,position*-1);
  multiplyMatrix(mat);
}

//! set projection
void GL2Renderer::projection(float angle, float aspect, float nearplane, float farplane){ 
  GLfloat ymax = nearplane * (GLfloat)tan(angle * 3.1415962f / 360.0);
  GLfloat ymin = -ymax;
  GLfloat xmin = ymin * aspect;
  GLfloat xmax = ymax * aspect;
  mMatrix[Projection] = CGE::Matrix(CGE::Matrix::Perspective, xmin, xmax, ymin, ymax, nearplane, farplane);
}

void GL2Renderer::ortho(float left, float right, float bottom, float top, float nearp, float farp){
  mMatrix[Projection] = CGE::Matrix(CGE::Matrix::Ortho, left, right, bottom, top, nearp, farp);
}

//! reset modelview matrix
void GL2Renderer::resetModelView(){
  mMatrix[mMatrixMode] = Matrix(Matrix::Identity);
}

//! translate
void GL2Renderer::translate(float x, float y, float z){
  mMatrix[mMatrixMode] *= Matrix(Matrix::Translation, Vec3f(x, y, z));
}

//! scale
void GL2Renderer::scale(float x, float y, float z){
  mMatrix[mMatrixMode] *= Matrix(Matrix::Scale, Vec3f(x, y, z));
}

//! rotate
void GL2Renderer::rotate(float angle, float x, float y, float z){
  mMatrix[mMatrixMode] *= Matrix(Matrix::Rotation, Vec3f(x, y, z), angle/180*(float)M_PI);
}

//! set rendermode
void GL2Renderer::renderMode(RendMode rm){
  /*if (rm == Filled){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else if(rm == Wireframe){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }*/
}

//! set blending mode
void GL2Renderer::blendFunc(BlendType src, BlendType dest){
  int srcfl = 0;
  int destfl = 0;
  switch(src){
    case BLEND_ONE:
      srcfl = GL_ONE;
      break;
    case BLEND_SRC_ALPHA:
      srcfl = GL_SRC_ALPHA;
      break;
    default:
      break;
  }
  switch(dest){
    case BLEND_ONE:
      destfl = GL_ONE;
      break;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      destfl = GL_ONE_MINUS_SRC_ALPHA;
      break;
    default:
      break;
  }
  glBlendFunc(srcfl, destfl);
}

//! enable blending
void GL2Renderer::enableBlend(const bool flag){
  if (flag)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
}

void GL2Renderer::enableBackFaceCulling(const bool flag){
  if (flag)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
}

//! enable texturing
void GL2Renderer::enableTexturing(const bool flag){
  int texEn = ((GL2Shader*)Shader::getCurrentShader())->getUniformLocation(Shader::FRAGMENT_SHADER, "textureEnabled");
  if (flag)
    ((GL2Shader*)Shader::getCurrentShader())->uniform(texEn, 1);
  else
    ((GL2Shader*)Shader::getCurrentShader())->uniform(texEn, 0);
}

// enable lighting
void GL2Renderer::enableLighting(const bool flag){
  if (flag && Shader::getCurrentShader() != mLightShader){
    Shader::getCurrentShader()->deactivate();
    mLightShader->activate();
  }
  else if (!flag && Shader::getCurrentShader() != mShader){
    Shader::getCurrentShader()->deactivate();
    mShader->activate();
  }
}

//! enable depth test
void GL2Renderer::enableDepthTest(const bool flag){
  if (flag)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
}

//! enable depth write
void GL2Renderer::enableDepthWrite(bool flag){
  glDepthMask(flag ? GL_TRUE : GL_FALSE);
}

//! set color
void GL2Renderer::setColor(float r, float g, float b, float a){
  glVertexAttrib4f(1, r, g, b, a);
}

//! set color
void GL2Renderer::setColor(const Color* c){
  glVertexAttrib4fv(1, c->array);
}

//! push matrix
void GL2Renderer::pushMatrix(){
  mMatrixStack[mMatrixMode].push(mMatrix[mMatrixMode]);
}

//! pop matrix
void GL2Renderer::popMatrix(){
  mMatrix[mMatrixMode] = mMatrixStack[mMatrixMode].top();
  mMatrixStack[mMatrixMode].pop();
}

//! multiply matrix
void GL2Renderer::multiplyMatrix(const CGE::Matrix& mat){
  mMatrix[mMatrixMode] *= mat;
}

//! set material
/*void GL2Renderer::setMaterial(const Material& mat){
  Color diff = mat.getDiffuse();
  diff.a = mat.getOpacity();
  if (mat.getOpacity() != 1.0f)
    enableBlend(true);
  else
    enableBlend(false);
  setColor(&diff);
  Texture const* tex = mat.getDiffuseTex();
  if (tex){
    enableTexturing(true);
    tex->activate();
  }
  else
    enableTexturing(false);
}*/

//! get the viewport
void GL2Renderer::getViewport(int view[4]){
  glGetIntegerv(GL_VIEWPORT, view);
}

//! get a matrix
Matrix GL2Renderer::getMatrix(MatrixType mt){
  if (mt == MVP){
    return mMatrix[Projection] * mMatrix[Modelview];
  }
  else if (mt == MatNormal){
    return mMatrix[Modelview].inverse().transpose();
  }
  return mMatrix[mt];
}

void GL2Renderer::swapBuffers(){
#ifdef WIN32
  SwapBuffers(hDC_);
#endif
#if defined UNIX && !defined QNX
  X11Window* win = static_cast<X11Window*>(win_);
  glXSwapBuffers(win->getDisplay(), win->getWindow());
#endif
}

void GL2Renderer::enableLight(short number, bool flag){
  /*if (flag)
    glEnable(GL_LIGHT0+number);
  else
    glDisable(GL_LIGHT0+number);*/
}

void GL2Renderer::setLight(int number, const Light& lit){
  Shader* shdr = Shader::getCurrentShader();
  if (!shdr)
    return;
  Vec4f pos = lit.getPosition();
  pos = mMatrix[mMatrixMode] * pos;
  Light trans(lit.getType(), pos);
  shdr->applyLight(number, trans);
}

void GL2Renderer::switchMatrixStack(MatrixType type){
  mMatrixMode = type;
}


