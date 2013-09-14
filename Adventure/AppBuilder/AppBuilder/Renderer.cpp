#include "Renderer.h"

using namespace adv;

TR_CHANNEL(ADV_Renderer)

RND_CLS* AdvRenderer::mInstance = NULL;

void AdvRenderer::init(){
#ifdef RENDER_GL2
  mInstance = new GL2Renderer();
#else
  mInstance = new GL1Renderer();
#endif
}

void AdvRenderer::deinit(){
  delete mInstance;
}

#ifdef RENDER_GL2

static const char vertexshadersource[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"\n"
"uniform mat4 matrices[3];\n"
"\n"
"varying vec4 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = matrices[2]*texCoord;\n"
"  gl_Position = matrices[1]*matrices[0]*vec4(position, 1.0);\n"
"}\n"
"";

static const char fragmentshadersource[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec4 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform int textureEnabled;\n"
"uniform vec4 constColor;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  if (textureEnabled == 1)\n"
"    color = texture2D(texture, tex_coord.st);\n"
"  color*=constColor;\n"
"  gl_FragColor = color;\n"
"}\n"
"";

GL2Renderer::GL2Renderer() : mMatrixMode(MM_MODELVIEW){
  for (int i = 0; i < 3; ++i){
    mMatrix[i] = CGE::Matrix(CGE::Matrix::Identity);
  }
  initShaders();
}

void GL2Renderer::initShaders(){
  mShader.addShader(GL_VERTEX_SHADER, vertexshadersource, sizeof(vertexshadersource));
  mShader.addShader(GL_FRAGMENT_SHADER, fragmentshadersource, sizeof(fragmentshadersource));
  mShader.linkShaders();
  mShader.activate();
  mAttribLocs[ATTR_VERTEX_ARRAY] = mShader.getAttribLocation("position");
  mAttribLocs[ATTR_TEXTURE_COORD_ARRAY] = mShader.getAttribLocation("texCoord");
  mMatricesLoc = mShader.getUniformLocation("matrices");
  mTextureLoc = mShader.getUniformLocation("texture");
  mTextureEnabledLoc = mShader.getUniformLocation("textureEnabled");
  mColorLoc = mShader.getUniformLocation("constColor");
  mShader.uniform(mTextureLoc, 0);
  mShader.uniform(mColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
}

#endif
