#define _USE_MATH_DEFINES
#include "LightPrepass.h"

#include <renderer/scene.h>
#include <system/engine.h>
#include <window/window.h>
#include <renderer/camera.h>
#include <renderer/drawcontext.h>
#include <renderer/renderdevice.h>
#include <renderer/uniformstream.h>

#include <cstring>

using namespace CGE;

TR_CHANNEL(CGE_LightPrepass);

LightPrepassRenderer::LightPrepassRenderer(){
}

LightPrepassRenderer::~LightPrepassRenderer(){
}

namespace{

//! the uniform buffer slot of the CGE_PrepassLight block (prepass_light.frag)
const int SLOT_PREPASS_LIGHT = 4;

//! the CGE_PrepassLight block (std140)
struct PrepassLightUniforms{
  Matrix inverseProjection = Matrix(Matrix::Identity);
  //! x: 1 if clip-space depth runs from 0 to 1
  float clipDepth[4] = {0, 0, 0, 0};
  float position[4] = {0, 0, 0, 0};
  float direction[4] = {0, 0, 0, 0};
  float color[4] = {0, 0, 0, 0};
  float attenuation[4] = {0, 0, 0, 0};
};
static_assert(sizeof(PrepassLightUniforms) == 144, "PrepassLightUniforms must match CGE_PrepassLight");

//! the G-buffer's colour attachments
enum GBufferAttachment{
  GBUFFER_NORMAL,
  GBUFFER_DIFFUSE,
  GBUFFER_SPECULAR,
};

}

struct LightPrepassRenderer::DeviceResources{
  std::unique_ptr<Gfx::Shader> gbufferShader;
  std::unique_ptr<Gfx::Shader> lightShader;
  std::unique_ptr<Gfx::Shader> composeShader;
  std::unique_ptr<Gfx::Pipeline> lightPipeline;
  std::unique_ptr<Gfx::Pipeline> composePipeline;
  //! for texelFetch, which ignores the filters
  std::unique_ptr<Gfx::Sampler> sampler;
  std::unique_ptr<Gfx::RenderTarget> gbuffer;
  std::unique_ptr<Gfx::RenderTarget> lightBuffer;
};

bool LightPrepassRenderer::createResources(Gfx::DrawContext& ctx){
  using namespace Gfx;
  TR_USE(CGE_LightPrepass);
  RenderDevice& device = ctx.getDevice();
  if (device.getCaps().maxColorAttachments < 3){
    TR_ERROR("The light prepass needs 3 colour attachments");
    return false;
  }
  mDevice = std::make_unique<DeviceResources>();

  ShaderDesc desc;
  desc.name = "engine/prepass_gbuffer";
  if (ShaderLibrary::loadDesc("engine/lit.vert", "engine/prepass_gbuffer.frag", desc))
    mDevice->gbufferShader = device.createShader(desc);

  desc = ShaderDesc();
  desc.name = "engine/prepass_light";
  if (ShaderLibrary::loadDesc("engine/fullscreen.vert", "engine/prepass_light.frag", desc)){
    desc.uniformBlocks.push_back({"CGE_PrepassLight", SLOT_PREPASS_LIGHT});
    desc.textures = {{"cge_gbufferNormal", 0}, {"cge_gbufferDepth", 1}};
    mDevice->lightShader = device.createShader(desc);
  }

  desc = ShaderDesc();
  desc.name = "engine/prepass_compose";
  if (ShaderLibrary::loadDesc("engine/fullscreen.vert", "engine/prepass_compose.frag", desc)){
    desc.textures = {{"cge_lightBuffer", 0}, {"cge_gbufferDiffuse", 1}, {"cge_gbufferSpecular", 2}, {"cge_gbufferDepth", 3}};
    mDevice->composeShader = device.createShader(desc);
  }
  if (!mDevice->gbufferShader || !mDevice->lightShader || !mDevice->composeShader){
    mDevice.reset();
    return false;
  }

  //every light adds to the light buffer
  PipelineDesc pipeline;
  pipeline.shader = mDevice->lightShader.get();
  pipeline.blend.enabled = true;
  pipeline.blend.srcColor = pipeline.blend.dstColor = BlendFactor::One;
  pipeline.blend.srcAlpha = pipeline.blend.dstAlpha = BlendFactor::One;
  pipeline.name = "prepass light";
  mDevice->lightPipeline = device.createPipeline(pipeline);

  //the composition writes the G-buffer's depth for the transparent surfaces
  pipeline = PipelineDesc();
  pipeline.shader = mDevice->composeShader.get();
  pipeline.depth.test = true;
  pipeline.depth.write = true;
  pipeline.depth.compare = Gfx::CompareFunc::AlwaysPass;
  pipeline.name = "prepass compose";
  mDevice->composePipeline = device.createPipeline(pipeline);

  SamplerDesc sampler;
  sampler.minFilter = sampler.magFilter = Filter::Nearest;
  sampler.addressU = sampler.addressV = AddressMode::ClampToEdge;
  mDevice->sampler = device.createSampler(sampler);
  if (!mDevice->lightPipeline || !mDevice->composePipeline || !mDevice->sampler){
    mDevice.reset();
    return false;
  }
  return true;
}

void LightPrepassRenderer::releaseResources(Gfx::DrawContext& ctx){
  if (!mDevice)
    return;
  ctx.forgetShader(mDevice->gbufferShader.get());
  mDevice.reset();
}

bool LightPrepassRenderer::createTargets(Gfx::RenderDevice& device, int width, int height){
  using namespace Gfx;
  if (mDevice->gbuffer && mDevice->gbuffer->getDesc().width == width && mDevice->gbuffer->getDesc().height == height)
    return mDevice->lightBuffer != nullptr;
  mDevice->gbuffer.reset();
  mDevice->lightBuffer.reset();
  RenderTargetDesc desc;
  desc.width = width;
  desc.height = height;
  //normal and shininess, diffuse colour, specular colour (see prepass_gbuffer.frag)
  desc.colorFormats = {Format::RGBA8, Format::RGBA8, Format::RGBA8};
  desc.depthFormat = Format::Depth24;
  mDevice->gbuffer = device.createRenderTarget(desc);
  desc.colorFormats = {Format::RGBA8};
  desc.depthFormat = Format::Undefined;
  mDevice->lightBuffer = device.createRenderTarget(desc);
  return mDevice->gbuffer && mDevice->lightBuffer;
}

void LightPrepassRenderer::render(Gfx::DrawContext& ctx, const Gfx::PassDesc& output){
  using namespace Gfx;
  if (!mScene || !mDevice)
    return;
  RenderDevice& device = ctx.getDevice();
  int width = output.target ? output.target->getDesc().width : device.getWidth();
  int height = output.target ? output.target->getDesc().height : device.getHeight();
  if (width <= 0 || height <= 0 || !createTargets(device, width, height))
    return;
  Camera* cam = mScene->getActiveCam();
  if (cam)
    cam->activate(ctx);

  //the G-buffer of the opaque surfaces
  PassDesc pass;
  pass.target = mDevice->gbuffer.get();
  pass.clearColor = {0, 0, 0, 0};
  if (!device.beginPass(pass))
    return;
  BlendMode blend = ctx.getBlend();
  ctx.setBlend(BlendMode::Opaque);
  ctx.setDepth(true, true);
  ctx.setLitShader(mDevice->gbufferShader.get());
  mScene->render(ctx, Mesh::DRAW_OPAQUE);
  ctx.setLitShader(nullptr);
  device.endPass();

  //the lights
  pass = PassDesc();
  pass.target = mDevice->lightBuffer.get();
  pass.clearColor = {0, 0, 0, 0};
  pass.depthLoad = LoadAction::DontCare;
  if (!device.beginPass(pass))
    return;
  device.setPipeline(mDevice->lightPipeline.get());
  device.setTexture(0, mDevice->gbuffer->getColorTexture(GBUFFER_NORMAL), mDevice->sampler.get());
  device.setTexture(1, mDevice->gbuffer->getDepthTexture(), mDevice->sampler.get());
  PrepassLightUniforms light;
  light.inverseProjection = ctx.getProjection().inverse();
  light.clipDepth[0] = device.getClipSpace().depthZeroToOne ? 1.0f : 0.0f;
  LightUniforms viewLight;
  for (Light* l : mScene->getLights()){
    DrawContext::setLightUniforms(viewLight, 0, *l, ctx.getView());
    memcpy(light.position, viewLight.position[0], sizeof(light.position));
    memcpy(light.direction, viewLight.direction[0], sizeof(light.direction));
    memcpy(light.color, viewLight.color[0], sizeof(light.color));
    memcpy(light.attenuation, viewLight.attenuation[0], sizeof(light.attenuation));
    if (ctx.getUniformStream().bind(SLOT_PREPASS_LIGHT, light))
      device.draw(3);
  }
  device.endPass();

  //the lit surfaces into the output, then the transparent ones
  if (!device.beginPass(output))
    return;
  device.setPipeline(mDevice->composePipeline.get());
  device.setTexture(0, mDevice->lightBuffer->getColorTexture(0), mDevice->sampler.get());
  device.setTexture(1, mDevice->gbuffer->getColorTexture(GBUFFER_DIFFUSE), mDevice->sampler.get());
  device.setTexture(2, mDevice->gbuffer->getColorTexture(GBUFFER_SPECULAR), mDevice->sampler.get());
  device.setTexture(3, mDevice->gbuffer->getDepthTexture(), mDevice->sampler.get());
  device.draw(3);
  ctx.setBlend(BlendMode::Alpha);
  mTransRend.render(ctx, Mesh::DRAW_TRANSPARENT);
  ctx.setBlend(blend);
  device.endPass();
}
