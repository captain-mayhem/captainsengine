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
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"uniform mat4 normalmat;\n"
"\n"
"varying vec3 vnormal;\n"
"varying vec2 uvcoord;\n"
"\n"
"void main(){\n"
"  vnormal = normalize((normalmat * vec4(normal, 0.0)).xyz);\n"
"  uvcoord = texcoord;\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"}\n"
"";

static char const * fs_src_base =
"uniform sampler2D texture;\n"
"\n"
"varying vec4 vnormal;\n"
"varying vec2 uvcoord;\n"
"\n"
"uniform int textureEnabled;\n"
"uniform vec4 matDiffuse;\n"
"uniform vec4 matSpecular;\n"
"uniform float matShininess;\n"
"\n"
"void main(){\n"
"  gl_FragData[0].rgb = normalize(vnormal);\n"
"  gl_FragData[0].a = matShininess;\n"
"  \n"
"  vec4 color = matDiffuse;\n"
"  if (textureEnabled)\n"
"    color *= texture2D(texture, uvcoord);\n"
"  gl_FragData[1] = color;\n"
"  gl_FragData[2] = matSpecular;\n"
"  \n"
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
"  float shininess = color.a;\n"
"  float depth = texture2D(depthTex, uvcoord).r;\n"
"  depth = camNear * camFar / (camFar - depth * (camFar-camNear));\n"
"  vec3 vpos = eyeDir*depth;\n"
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
"      att = 1.0/(1.0+lightAttenuation*pow(lightDist, 2.0));\n"
"    }\n"
"  }\n"
"  vec3 refl = normalize(reflect(-lightvec, normal));\n"
"  float NL = max(dot(normal,lightvec), 0.0);\n"
"  float spec = 0.0;\n"
"  if (NL > 0.0)\n"
"    spec = pow(max(dot(refl, eye), 0.0), shininess);\n"
"  \n"
"  vec3 diffuse = lightColor.rgb*NL*att;\n"
//"  vec3 specular = lightColor.rgb*spec*NL*att;\n"
//"  gl_FragColor.a = 0.299*specular.r+0.587*specular.g+0.114*specular.b;\n"
"  \n"
//"  float showdepth = (depth - camNear)/(camFar-camNear);\n"
"  gl_FragColor.rgb = diffuse;\n"
"  gl_FragColor.a = spec*att;\n"
"}\n"
"";

static char const * vs_src_compositing =
"attribute vec3 pos;\n"
"attribute vec2 texcoord;\n"
"\n"
"varying vec2 uvcoord;\n"
"\n"
"void main(){\n"
"  uvcoord = vec2(texcoord.x, 1-texcoord.y);\n"
"  gl_Position = vec4(2*pos.x, 2*pos.y, 2*pos.z, 1.0);\n"
"}\n"
"";

static char const * fs_src_compositing =
"uniform sampler2D lightBuffer;\n"
"uniform sampler2D diffuseTex;\n"
"uniform sampler2D specTex;\n"
"\n"
"varying vec2 uvcoord;\n"
"\n"
"void main(){\n"
"  vec4 diffuse = texture2D(diffuseTex, uvcoord);\n"
"  if (diffuse.a == 0.0)\n"
"    discard;\n"
"  vec4 specular = texture2D(specTex, uvcoord);\n"
"  vec4 lcolor = texture2D(lightBuffer, uvcoord);\n"
"  vec3 chroma = lcolor.rgb/(0.299*lcolor.r+0.587*lcolor.g+0.114*lcolor.b+0.0);\n"
"  vec3 spec = chroma*lcolor.aaa;\n"
"  gl_FragColor.rgb = diffuse.rgb*lcolor.rgb + specular.rgb*spec;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

void LightPrepassRenderer::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  mBaseShader = rend->createShader();
  mBaseShader->registerMatCB(applyMaterial, this);
  mBaseShader->addShader(Shader::VERTEX_SHADER, vs_src_base, 0);
  mBaseShader->addShader(Shader::FRAGMENT_SHADER, fs_src_base, 0);
 
  mBaseShader->linkShaders();
  mBaseShader->activate();
  mBaseShader->syncMatrix("mvp", CGE::MVP);
  mBaseShader->syncMatrix("normalmat", CGE::MatNormal);
  mTexEnabledLoc = mBaseShader->getUniformLocation(Shader::FRAGMENT_SHADER, "textureEnabled");
  mColorLoc = mBaseShader->getUniformLocation(Shader::FRAGMENT_SHADER, "matDiffuse");
  mSpecularLoc = mBaseShader->getUniformLocation(Shader::FRAGMENT_SHADER, "matSpecular");
  mShininessLoc = mBaseShader->getUniformLocation(Shader::FRAGMENT_SHADER, "matShininess");
  mBaseShader->deactivate();

  mGBuffer = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  mGBuffer->activate();
  mGBuffer->addTexture(Texture::FLOAT);//normal + shininess
  mGBuffer->addTexture(Texture::RGBA);//diffuse
  mGBuffer->addTexture(Texture::DEPTH);//depth
  mGBuffer->addTexture(Texture::RGBA);//specular
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
  mLightShader->uniform(mLightShader->getUniformLocation(Shader::FRAGMENT_SHADER, "depthTex"), 2);
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

  mLightBuffer = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  mLightBuffer->activate();
  mLightBuffer->addTexture(Texture::RGBA);
  mLightBuffer->addRenderbuffer(Texture::DEPTH);
  mLightBuffer->isComplete();
  mLightBuffer->deactivate();

  mCompositingShader = rend->createShader();
  mCompositingShader->addShader(Shader::VERTEX_SHADER, vs_src_compositing, 0);
  mCompositingShader->addShader(Shader::FRAGMENT_SHADER, fs_src_compositing, 0);
  mCompositingShader->linkShaders();
  mCompositingShader->activate();
  mCompositingShader->uniform(mCompositingShader->getUniformLocation(Shader::FRAGMENT_SHADER, "lightBuffer"), 0);
  mCompositingShader->uniform(mCompositingShader->getUniformLocation(Shader::FRAGMENT_SHADER, "diffuseTex"), 1);
  mCompositingShader->uniform(mCompositingShader->getUniformLocation(Shader::FRAGMENT_SHADER, "specTex"), 2);
  mCompositingShader->deactivate();

  mCompositingBuffer = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  mCompositingBuffer->activate();
  mCompositingBuffer->addTexture(Texture::RGBA);
  mCompositingBuffer->addTexture(*mGBuffer->getTexture(2));
  mCompositingBuffer->isComplete();
  mCompositingBuffer->deactivate();
}

void LightPrepassRenderer::deinit(){
  delete mBaseShader;
  delete mGBuffer;
  delete mLightShader;
  delete mLightBuffer;
  delete mCompositingShader;
  delete mCompositingBuffer;
}

void LightPrepassRenderer::render(){
  if (!mScene)
    return;
  Renderer* rend = Engine::instance()->getRenderer();
  mBaseShader->activate();
  mGBuffer->activate();
  rend->setClearColor(Vec4f(0, 0, 0, 0));
  rend->clear(ZBUFFER | COLORBUFFER);
  mScene->render(Mesh::DRAW_OPAQUE);
  mGBuffer->deactivate();
  mBaseShader->deactivate();

  mLightShader->activate();
  mLightShader->uniform(mCamNearLoc, mScene->getActiveCam()->getFrustum().getNearDist());
  mLightShader->uniform(mCamFarLoc, mScene->getActiveCam()->getFrustum().getFarDist());
  float npy = tanf(mScene->getActiveCam()->getFrustum().getAngle() / 180.0f*(float)M_PI / 2.0f);
  mLightShader->uniform(mNearPlaneSizeLoc, npy*mScene->getActiveCam()->getFrustum().getRatio(), npy);
  
  mLightBuffer->activate();
  rend->enableBlend(true);
  rend->blendFunc(BLEND_ONE, BLEND_ONE); //additive without alpha
  rend->clear(ZBUFFER | COLORBUFFER);
  for (unsigned i = 0; i < mScene->getLights().size(); ++i){
    Light* l = mScene->getLights()[i];
    rend->setLight(0, *l);
    mGBuffer->drawFullscreen(false);
  }
  rend->enableBlend(false);
  mLightBuffer->deactivate();
  
  mLightShader->deactivate();

  mCompositingBuffer->activate();
  rend->clear(COLORBUFFER);
  mCompositingShader->activate();
  mGBuffer->getTexture(1)->activate(1);
  mGBuffer->getTexture(3)->activate(2);
  mLightBuffer->drawFullscreen(false);
  mCompositingShader->deactivate();

  rend->blendFunc(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableDepthTest(true);
  mTransRend.render(Mesh::DRAW_TRANSPARENT);
  mCompositingBuffer->deactivate();

  mCompositingBuffer->drawFullscreen(true);
}

void LightPrepassRenderer::applyMaterial(Shader* shader, Material const& mat, void* userdata){
  LightPrepassRenderer* rend = (LightPrepassRenderer*)userdata;
  shader->lockUniforms(Shader::FRAGMENT_SHADER);
  Color diff = mat.getDiffuse();
  diff.a = mat.getOpacity();
  shader->uniform(rend->mColorLoc, diff);
  shader->uniform(rend->mAmbientLoc, mat.getAmbient());
  shader->uniform(rend->mShininessLoc, mat.getPower());
  shader->uniform(rend->mSpecularLoc, mat.getSpecular());
  Texture const* tex = mat.getDiffuseTex();
  if (!tex)
    tex = mat.getOpacityTex();
  if (tex){
    shader->uniform(rend->mTexEnabledLoc, 1);
    tex->activate();
  }
  else
    shader->uniform(rend->mTexEnabledLoc, 0);
  shader->unlockUniforms(Shader::FRAGMENT_SHADER);
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