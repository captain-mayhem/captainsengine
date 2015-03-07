#include "LightPrepass.h"

#include <renderer/scene.h>
#include <renderer/rendertarget.h>
#include <renderer/renderer.h>
#include <system/engine.h>
#include <window/window.h>
#include <renderer/shader.h>

using namespace CGE;

static char const * vs_src_base =
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 normalmat;\n"
"\n"
"varying vec3 vnormal;\n"
"\n"
"void main(){\n"
"  vnormal = normalize((normalmat * vec4(normal, 0.0)).xyz);\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"}\n"
"";

static char const * fs_src_base =
"uniform sampler2D texture;\n"
"\n"
"varying vec4 vnormal;\n"
"\n"
"void main(){\n"
"  gl_FragColor.rgb = normalize(vnormal);\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

void LightPrepassRenderer::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  mBaseShader = rend->createShader();
  mBaseShader->addShader(Shader::VERTEX_SHADER, vs_src_base, 0);
  mBaseShader->addShader(Shader::FRAGMENT_SHADER, fs_src_base, 0);
  /*mBaseShader->bindAttribLocation(0, "pos");
  mBaseShader->bindAttribLocation(1, "color");
  mBaseShader->bindAttribLocation(2, "texcoord");
  mBaseShader->bindAttribLocation(3, "normal");*/
  mBaseShader->linkShaders();
  mBaseShader->activate();
  mBaseShader->syncMatrix("mvp", CGE::MVP);
  mBaseShader->syncMatrix("normalmat", CGE::MatNormal);
  mBaseShader->deactivate();

  mGBuffer = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  if (mGBuffer){
    mGBuffer->activate();
    mGBuffer->addTexture(CGE::Texture::RGBA);
    mGBuffer->addTexture(CGE::Texture::DEPTH);
    mGBuffer->isComplete();
    mGBuffer->deactivate();
  }
}

void LightPrepassRenderer::deinit(){
  delete mBaseShader;
  delete mGBuffer;
}

void LightPrepassRenderer::render(){
  if (!mScene)
    return;
  mBaseShader->activate();
  mScene->render();
  mBaseShader->deactivate();
  //mScene->render();
}