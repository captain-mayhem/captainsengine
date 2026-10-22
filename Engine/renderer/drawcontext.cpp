#include "drawcontext.h"

#include <map>
#include <tuple>

#include "light.h"
#include "renderdevice.h"
#include "uniformstream.h"
#include "../io/Tracing.h"
#include "../math/matrixstack.h"

using namespace CGE;
using namespace CGE::Gfx;

TR_CHANNEL(CGE_DrawContext);

namespace{

const int BUILTIN_COUNT = 4;

//! what a cached pipeline was made for
struct PipelineKey{
  Shader* shader;
  //! the vertex layout's numbers
  std::vector<int> layout;
  PrimitiveType primitive;
  BlendMode blend;
  bool depthTest;
  bool depthWrite;
  CompareFunc depthCompare;
  bool colorWrite;
  CullMode cull;

  bool operator<(const PipelineKey& other) const{
    return std::tie(shader, layout, primitive, blend, depthTest, depthWrite, depthCompare, colorWrite, cull) <
      std::tie(other.shader, other.layout, other.primitive, other.blend, other.depthTest, other.depthWrite,
        other.depthCompare, other.colorWrite, other.cull);
  }
};

std::vector<int> layoutKey(const VertexLayout& layout){
  std::vector<int> key;
  for (const VertexAttribute& attr : layout.attributes){
    key.push_back(attr.location);
    key.push_back((int)attr.format);
    key.push_back(attr.bufferSlot);
    key.push_back(attr.offset);
  }
  key.push_back(-1);
  for (const VertexBufferLayout& buffer : layout.buffers){
    key.push_back(buffer.stride);
    key.push_back(buffer.perInstance ? 1 : 0);
  }
  return key;
}

void copy4(float* dest, float x, float y, float z, float w){
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
  dest[3] = w;
}

}

struct DrawContext::Impl{
  explicit Impl(RenderDevice& dev) : device(dev), stream(dev){}

  //! a range of the stream, valid while the stream's generation is the same
  struct Bound{
    UniformRange range;
    uint64_t generation = 0;
    bool valid(const UniformStream& stream) const {return range.buffer && generation == stream.getGeneration();}
  };

  //! pushes data unless it is unchanged and still in the stream
  bool bind(int slot, const void* data, size_t size, bool& dirty, Bound& bound){
    if (dirty || !bound.valid(stream)){
      bound.range = stream.push(data, size);
      bound.generation = stream.getGeneration();
      if (!bound.range.buffer)
        return false;
      dirty = false;
    }
    device.setUniformBuffer(slot, bound.range.buffer, bound.range.offset, bound.range.size);
    return true;
  }

  //! the lights in view space
  void updateLightData(){
    lightData = LightUniforms();
    int count = (int)lights.size();
    if (count > MAX_LIGHTS)
      count = MAX_LIGHTS;
    lightData.count[0] = count;
    for (int i = 0; i < count; ++i)
      DrawContext::setLightUniforms(lightData, i, lights[i], frame.view);
  }

  RenderDevice& device;
  UniformStream stream;

  FrameUniforms frame;
  bool frameDirty = true;
  Bound frameBound;

  MatrixStack model;
  Color color;

  MaterialUniforms material;
  bool materialDirty = true;
  Bound materialBound;

  std::vector<Light> lights;
  LightUniforms lightData;
  //! the lights or the view changed
  bool lightsDirty = true;
  Bound lightsBound;
  Shader* litShader = nullptr;

  Texture* texture = nullptr;
  Sampler* sampler = nullptr;
  std::unique_ptr<Sampler> defaultSampler;
  BlendMode blend = BlendMode::Opaque;
  bool depthTest = false;
  bool depthWrite = false;
  CompareFunc depthCompare = CompareFunc::LessEqual;
  bool colorWrite = true;
  CullMode cull = CullMode::Off;

  std::unique_ptr<Shader> builtins[BUILTIN_COUNT];
  std::map<PipelineKey, std::unique_ptr<Pipeline>> pipelines;
};

DrawContext::DrawContext(RenderDevice& device) : impl_(std::make_unique<Impl>(device)){
}

DrawContext::~DrawContext(){
  //the pipelines refer to the shaders
  impl_->pipelines.clear();
}

RenderDevice& DrawContext::getDevice() const{
  return impl_->device;
}

UniformStream& DrawContext::getUniformStream(){
  return impl_->stream;
}

void DrawContext::setProjection(const Matrix& projection){
  impl_->frame.projection = projection;
  impl_->frameDirty = true;
}

const Matrix& DrawContext::getProjection() const{
  return impl_->frame.projection;
}

void DrawContext::setView(const Matrix& view){
  impl_->frame.view = view;
  impl_->frameDirty = true;
  impl_->lightsDirty = true;
}

const Matrix& DrawContext::getView() const{
  return impl_->frame.view;
}

void DrawContext::setTime(float seconds){
  impl_->frame.time[0] = seconds;
  impl_->frameDirty = true;
}

MatrixStack& DrawContext::model(){
  return impl_->model;
}

void DrawContext::setColor(float r, float g, float b, float a){
  impl_->color = Color(r, g, b, a);
}

void DrawContext::setColor(const Color& color){
  impl_->color = color;
}

const Color& DrawContext::getColor() const{
  return impl_->color;
}

void DrawContext::setMaterial(const Material& material){
  MaterialUniforms uniforms;
  const Color& diffuse = material.getDiffuse();
  copy4(uniforms.diffuse, diffuse.r, diffuse.g, diffuse.b, material.getOpacity());
  const Color& ambient = material.getAmbient();
  copy4(uniforms.ambient, ambient.r, ambient.g, ambient.b, ambient.a);
  const Color& specular = material.getSpecular();
  copy4(uniforms.specular, specular.r, specular.g, specular.b, material.getPower());
  setMaterial(uniforms);
}

void DrawContext::setMaterial(const MaterialUniforms& material){
  impl_->material = material;
  impl_->materialDirty = true;
}

void DrawContext::resetMaterial(){
  setMaterial(MaterialUniforms());
}

void DrawContext::setLights(const std::vector<Light*>& lights){
  impl_->lights.clear();
  for (Light* light : lights){
    if ((int)impl_->lights.size() == MAX_LIGHTS)
      break;
    impl_->lights.push_back(*light);
  }
  impl_->lightsDirty = true;
}

void DrawContext::clearLights(){
  impl_->lights.clear();
  impl_->lightsDirty = true;
}

int DrawContext::getNumLights() const{
  return (int)impl_->lights.size();
}

void DrawContext::setLightUniforms(LightUniforms& uniforms, int index, const Light& light, const Matrix& view){
  //positions with the view, directions with its inverse transpose, as the old renderers did
  Matrix normalView = view.inverse().transpose();
  Vec4f pos = light.getPosition();
  if (light.getType() == Light::Directional)
    pos = normalView*pos;
  else
    pos = view*pos;
  copy4(uniforms.position[index], pos.x, pos.y, pos.z, pos.w);
  Vec3f dir = light.getDirection();
  Vec4f viewDir = normalView*Vec4f(dir.x, dir.y, dir.z, 0);
  copy4(uniforms.direction[index], viewDir.x, viewDir.y, viewDir.z, light.getCutoff()/180.0f*3.14159265358979f);
  const Color& col = light.getColor();
  copy4(uniforms.color[index], col.r, col.g, col.b, col.a);
  copy4(uniforms.attenuation[index], light.getAttenuation(), 0, 0, 0);
}

void DrawContext::setLitShader(Shader* shader){
  impl_->litShader = shader;
}

void DrawContext::setTexture(Texture* texture, Sampler* sampler){
  impl_->texture = texture;
  impl_->sampler = sampler;
  impl_->materialDirty = true;
}

Gfx::Texture* DrawContext::getTexture() const{
  return impl_->texture;
}

Gfx::Sampler* DrawContext::getSampler() const{
  return impl_->sampler;
}

void DrawContext::setBlend(BlendMode mode){
  impl_->blend = mode;
}

BlendMode DrawContext::getBlend() const{
  return impl_->blend;
}

void DrawContext::setDepth(bool test, bool write, CompareFunc compare){
  impl_->depthTest = test;
  impl_->depthWrite = write;
  impl_->depthCompare = compare;
}

void DrawContext::setColorWrite(bool write){
  impl_->colorWrite = write;
}

void DrawContext::setCull(CullMode cull){
  impl_->cull = cull;
}

Shader* DrawContext::getShader(BuiltinShader shader){
  if (shader == BuiltinShader::Lit && impl_->litShader)
    return impl_->litShader;
  int idx = (int)shader;
  if (idx < 0 || idx >= BUILTIN_COUNT)
    return nullptr;
  if (!impl_->builtins[idx])
    impl_->builtins[idx] = ShaderLibrary::createBuiltin(impl_->device, shader);
  return impl_->builtins[idx].get();
}

Pipeline* DrawContext::getPipeline(Shader* shader, const VertexLayout& layout, PrimitiveType primitive){
  if (!shader)
    return nullptr;
  Impl& d = *impl_;
  PipelineKey key{shader, layoutKey(layout), primitive, d.blend, d.depthTest, d.depthWrite, d.depthCompare, d.colorWrite, d.cull};
  auto iter = d.pipelines.find(key);
  if (iter != d.pipelines.end())
    return iter->second.get();

  PipelineDesc desc;
  desc.shader = shader;
  desc.vertexLayout = layout;
  desc.primitive = primitive;
  switch (d.blend){
    case BlendMode::Opaque:
      break;
    case BlendMode::Alpha:
      desc.blend.enabled = true;
      desc.blend.srcColor = desc.blend.srcAlpha = BlendFactor::SrcAlpha;
      desc.blend.dstColor = desc.blend.dstAlpha = BlendFactor::OneMinusSrcAlpha;
      break;
    case BlendMode::Additive:
      desc.blend.enabled = true;
      desc.blend.srcColor = desc.blend.srcAlpha = BlendFactor::SrcAlpha;
      desc.blend.dstColor = desc.blend.dstAlpha = BlendFactor::One;
      break;
    case BlendMode::PremultipliedAlpha:
      desc.blend.enabled = true;
      desc.blend.srcColor = desc.blend.srcAlpha = BlendFactor::One;
      desc.blend.dstColor = desc.blend.dstAlpha = BlendFactor::OneMinusSrcAlpha;
      break;
    case BlendMode::Multiply:
      desc.blend.enabled = true;
      desc.blend.srcColor = desc.blend.srcAlpha = BlendFactor::DstColor;
      desc.blend.dstColor = desc.blend.dstAlpha = BlendFactor::Zero;
      break;
  }
  desc.blend.writeMask = d.colorWrite ? COLOR_ALL : 0;
  desc.depth.test = d.depthTest;
  desc.depth.write = d.depthWrite;
  desc.depth.compare = d.depthCompare;
  desc.raster.cull = d.cull;
  desc.name = "DrawContext " + shader->getName();
  std::unique_ptr<Pipeline> pipeline = d.device.createPipeline(desc);
  //a failed pipeline is cached too, so the error is reported once
  Pipeline* result = pipeline.get();
  d.pipelines[key] = std::move(pipeline);
  return result;
}

void DrawContext::forgetShader(Shader* shader){
  auto& pipelines = impl_->pipelines;
  for (auto iter = pipelines.begin(); iter != pipelines.end();){
    if (iter->first.shader == shader)
      iter = pipelines.erase(iter);
    else
      ++iter;
  }
  if (impl_->litShader == shader)
    impl_->litShader = nullptr;
}

bool DrawContext::prepare(Shader* shader, const VertexLayout& layout, PrimitiveType primitive){
  TR_USE(CGE_DrawContext);
  Impl& d = *impl_;
  if (!d.device.isInPass()){
    TR_ERROR("prepare: not in a pass");
    return false;
  }
  Pipeline* pipeline = getPipeline(shader, layout, primitive);
  if (!pipeline)
    return false;
  d.device.setPipeline(pipeline);

  if (!d.bind(SLOT_FRAME, &d.frame, sizeof(d.frame), d.frameDirty, d.frameBound))
    return false;

  DrawUniforms draw;
  draw.model = d.model.top();
  copy4(draw.color, d.color.r, d.color.g, d.color.b, d.color.a);
  draw.normalMatrix = (d.frame.view*draw.model).inverse().transpose();
  if (!d.stream.bind(SLOT_DRAW, draw))
    return false;

  d.material.flags[0] = d.texture ? 1 : 0;
  if (!d.bind(SLOT_MATERIAL, &d.material, sizeof(d.material), d.materialDirty, d.materialBound))
    return false;
  if (d.lightsDirty)
    d.updateLightData();
  if (!d.bind(SLOT_LIGHTS, &d.lightData, sizeof(d.lightData), d.lightsDirty, d.lightsBound))
    return false;

  if (d.texture){
    Sampler* sampler = d.sampler;
    if (!sampler){
      if (!d.defaultSampler){
        SamplerDesc desc;
        desc.mipFilter = MipFilter::Linear;
        d.defaultSampler = d.device.createSampler(desc);
      }
      sampler = d.defaultSampler.get();
    }
    d.device.setTexture(SLOT_TEXTURE, d.texture, sampler);
  }
  return true;
}

bool DrawContext::prepare(BuiltinShader shader, const VertexLayout& layout, PrimitiveType primitive){
  return prepare(getShader(shader), layout, primitive);
}

void DrawContext::draw(BuiltinShader shader, Buffer* vertices, const VertexLayout& layout, PrimitiveType primitive,
  int vertexCount, int firstVertex){
  if (!vertices || !prepare(shader, layout, primitive))
    return;
  impl_->device.setVertexBuffer(0, vertices);
  impl_->device.draw(vertexCount, 1, firstVertex);
}

void DrawContext::drawIndexed(BuiltinShader shader, Buffer* vertices, const VertexLayout& layout, Buffer* indices,
  IndexType indexType, PrimitiveType primitive, int indexCount, int firstIndex){
  if (!vertices || !indices || !prepare(shader, layout, primitive))
    return;
  impl_->device.setVertexBuffer(0, vertices);
  impl_->device.setIndexBuffer(indices, indexType);
  impl_->device.drawIndexed(indexCount, 1, firstIndex);
}
