#define _USE_MATH_DEFINES
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
"uniform vec2 nearPlaneSize;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec3 eyeDir;\n"
"\n"
"void main(){\n"
"  uvcoord = vec2(texcoord.x, 1-texcoord.y);\n"
"  eyeDir = vec3(2.0*nearPlaneSize*uvcoord - nearPlaneSize, -1.0);\n"
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
"varying vec3 eyeDir;\n"
"\n"
"uniform vec4 lightPos;\n"
"uniform vec3 lightDir;\n"
"uniform vec4 lightColor;\n"
"uniform float lightCutoff;\n"
"uniform float lightAttenuation;\n"
"\n"
"void main(){\n"
"  vec4 color = texture2D(texture, uvcoord);\n"
"  vec3 normal = normalize(color.xyz);\n"
"  float depth = texture2D(depthTex, uvcoord).r;\n"
"  depth = camNear * camFar / (camFar - depth * (camFar-camNear));\n"
"  vec3 vpos = normalize(eyeDir)*depth;\n"
"  vec3 eye = normalize(-vpos);\n"
"  \n"
"  vec3 lightvec;\n"
"  float att = 1.0;\n"
"  if (lightPos.w == 0.0)\n"
"    lightvec = normalize(lightPos.xyz);\n"
"  else{\n"
"    lightvec = normalize(lightPos.xyz - vpos);\n"
"    float lightAngle = acos(dot(-lightvec, normalize(lightDir)));\n"
"    if (lightAngle > lightCutoff){\n"
"      att = 0.0;\n"
"    }\n"
"    else{\n"
"      float lightDist = length(lightPos.xyz-vpos);\n"
"      att = 1.0/(1.0+lightAttenuation*pow(lightDist, 2));\n"
"    }\n"
"  }\n"
"  vec3 refl = normalize(reflect(-lightvec, normal));\n"
"  float NL = max(dot(normal,lightvec), 0.0);\n"
"  float spec = 0.0;\n"
"  if (NL > 0.0)\n"
"    spec = pow(max(dot(refl, eye), 0.0), 20);\n"
"  \n"
"  vec3 diffuse = lightColor.rgb*NL*att;\n"
//"  specular += lightColor[i].rgb*spec*att;\n"
//"  ambient += lightColor[i].rgb*matAmbient.rgb;\n"
"  \n"
"  float showdepth = (depth - camNear)/(camFar-camNear);\n"
"  gl_FragColor.rgb = vec3(uvcoord.x,uvcoord.y,showdepth);\n"
//"  gl_FragColor.rgb = vec3(showdepth);\n"
"  gl_FragColor.rgb = diffuse;\n"
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
  mLightShader->registerLightCB(applyLight, this);
  mLightShader->addShader(Shader::VERTEX_SHADER, vs_src_light, 0);
  mLightShader->addShader(Shader::FRAGMENT_SHADER, fs_src_light, 0);

  mLightShader->linkShaders();
  mLightShader->activate();
  mNearPlaneSizeLoc = mLightShader->getUniformLocation(Shader::VERTEX_SHADER, "nearPlaneSize");
  mLightShader->uniform(mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "texture"), 0);
  mLightShader->uniform(mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "depthTex"), 1);
  mCamFarLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "camFar");
  mCamNearLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "camNear");
  mLightPosLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "lightPos");
  mLightDirLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "lightDir");
  mLightCutoffLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "lightCutoff");
  mLightColorLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "lightColor");
  mLightAttenuationLoc = mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "lightAttenuation");
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
  float npy = tanf(mScene->getActiveCam()->getFrustum().getAngle()/180.0f*(float)M_PI/2.0f);
  mLightShader->uniform(mNearPlaneSizeLoc, npy*mScene->getActiveCam()->getFrustum().getRatio(), npy);
  
  rend->enableBlend(true);
  rend->blendFunc(BLEND_SRC_ALPHA, BLEND_ONE);
  for (unsigned i = 0; i < mScene->getLights().size(); ++i){
    Light* l = mScene->getLights()[i];
    rend->setLight(0, *l);
    mGBuffer->drawFullscreen(false);
  }
  rend->enableBlend(false);
  
  mLightShader->deactivate();
  //mGBuffer->getTexture(1)->deactivate(1);

  //rend->clear(ZBUFFER | COLORBUFFER);
  //mScene->render();
}

void LightPrepassRenderer::applyLight(Shader* shader, int number, Light const& light, void* userdata){
  LightPrepassRenderer* rend = (LightPrepassRenderer*)userdata;
  shader->lockUniforms(Shader::FRAGMENT_SHADER, 1);
  Vec4f pos = light.getPosition();
  shader->uniform(rend->mLightPosLoc, pos.x, pos.y, pos.z, pos.w);
  shader->uniform(rend->mLightDirLoc, light.getDirection());
  shader->uniform(rend->mLightCutoffLoc, light.getCutoff() / 180.0f*(float)M_PI);
  shader->uniform(rend->mLightAttenuationLoc, light.getAttenuation());
  shader->uniform(rend->mLightColorLoc, light.getColor());
  shader->unlockUniforms(Shader::FRAGMENT_SHADER, 1);
}