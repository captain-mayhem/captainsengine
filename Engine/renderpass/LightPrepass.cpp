#include "LightPrepass.h"

#include <renderer/scene.h>
#include <renderer/rendertarget.h>
#include <renderer/renderer.h>
#include <system/engine.h>
#include <window/window.h>
#include <renderer/shader.h>
#include <renderer/camera.h>

using namespace CGE;

static char const * vs_src_base =
"attribute vec3 pos;\n"
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

static char const * vs_src_light =
"attribute vec3 pos;\n"
"attribute vec2 texcoord;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 normalmat;\n"
"\n"
"varying vec2 uvcoord;\n"
"\n"
"void main(){\n"
"  uvcoord = vec2(texcoord.x, 1-texcoord.y);\n"
"  gl_Position = vec4(2*pos.x, 2*pos.y, 2*pos.z, 1.0);\n"
"}\n"
"";

static char const * fs_src_light =
"uniform sampler2D texture;\n"
"uniform sampler2D depthTex;\n"
"\n"
"uniform float camNear;\n"
"uniform float camFar;\n"
"\n"
"varying vec2 uvcoord;\n"
"\n"
"void main(){\n"
"  vec4 color = texture2D(texture, uvcoord);\n"
"  float depth = texture2D(depthTex, uvcoord).r;\n"
"  depth = camNear * camFar / (camFar - depth * (camFar-camNear));\n"
"  \n"
"  float showdepth = (depth - camNear)/(camFar-camNear);\n"
"  gl_FragColor.rgb = vec3(showdepth,showdepth,showdepth);\n"
//"  gl_FragColor.rgb = color.rgb;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

void LightPrepassRenderer::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  mBaseShader = rend->createShader();
  mBaseShader->addShader(Shader::VERTEX_SHADER, vs_src_base, 0);
  mBaseShader->addShader(Shader::FRAGMENT_SHADER, fs_src_base, 0);
 
  mBaseShader->linkShaders();
  mBaseShader->activate();
  mBaseShader->syncMatrix("mvp", CGE::MVP);
  mBaseShader->syncMatrix("normalmat", CGE::MatNormal);
  mBaseShader->deactivate();

  mGBuffer = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  mGBuffer->activate();
  mGBuffer->addTexture(CGE::Texture::RGBA);
  mGBuffer->addTexture(CGE::Texture::DEPTH);
  mGBuffer->isComplete();
  mGBuffer->deactivate();

  mLightShader = rend->createShader();
  mLightShader->addShader(Shader::VERTEX_SHADER, vs_src_light, 0);
  mLightShader->addShader(Shader::FRAGMENT_SHADER, fs_src_light, 0);

  mLightShader->linkShaders();
  mLightShader->activate();
  mLightShader->uniform(mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "texture"), 0);
  mLightShader->uniform(mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "depthTex"), 1);
  mCamFarLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "camFar");
  mCamNearLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "camNear");
  //mLightShader->syncMatrix("mvp", CGE::MVP);
  //mLightShader->syncMatrix("normalmat", CGE::MatNormal);
  mLightShader->deactivate();
}

void LightPrepassRenderer::deinit(){
  delete mBaseShader;
  delete mGBuffer;
  delete mLightShader;
}

void LightPrepassRenderer::render(){
  if (!mScene)
    return;
  Renderer* rend = Engine::instance()->getRenderer();
  mBaseShader->activate();
  mGBuffer->activate();
  rend->clear(ZBUFFER | COLORBUFFER);
  mScene->render();
  mGBuffer->deactivate();
  mBaseShader->deactivate();

  mLightShader->activate();
  mLightShader->uniform(mCamNearLoc, mScene->getActiveCam()->getFrustum().getNearDist());
  mLightShader->uniform(mCamFarLoc, mScene->getActiveCam()->getFrustum().getFarDist());
  mGBuffer->drawFullscreen(false);
  mLightShader->deactivate();
  //mGBuffer->getTexture(1)->deactivate(1);

  //rend->clear(ZBUFFER | COLORBUFFER);
  //mScene->render();
}