#include "nulldevice.h"

#include <vector>

using namespace CGE::Gfx;

namespace{

class NullBuffer : public Buffer{
public:
  explicit NullBuffer(const BufferDesc& desc) : Buffer(desc) {}
};

class NullTexture : public Texture{
public:
  explicit NullTexture(const TextureDesc& desc) : Texture(desc) {}
};

class NullSampler : public Sampler{
public:
  explicit NullSampler(const SamplerDesc& desc) : Sampler(desc) {}
};

class NullShader : public Shader{
public:
  explicit NullShader(const std::string& name) : Shader(name) {}
};

class NullPipeline : public Pipeline{
public:
  explicit NullPipeline(const PipelineDesc& desc) : Pipeline(desc) {}
};

class NullRenderTarget : public RenderTarget{
public:
  explicit NullRenderTarget(const RenderTargetDesc& desc) : RenderTarget(desc){
    TextureDesc tex;
    tex.width = desc.width;
    tex.height = desc.height;
    tex.sampled = desc.sampled;
    tex.renderTarget = true;
    for (Format fmt : desc.colorFormats){
      tex.format = fmt;
      colors_.push_back(std::make_unique<NullTexture>(tex));
    }
    if (desc.depthFormat != Format::Undefined){
      tex.format = desc.depthFormat;
      depth_ = std::make_unique<NullTexture>(tex);
    }
  }
  Texture* getColorTexture(int idx) const override{
    return idx >= 0 && idx < (int)colors_.size() ? colors_[idx].get() : nullptr;
  }
  Texture* getDepthTexture() const override {return depth_.get();}
private:
  std::vector<std::unique_ptr<NullTexture> > colors_;
  std::unique_ptr<NullTexture> depth_;
};

}

NullDevice::NullDevice(){
  frames_ = 0;
  draws_ = 0;
}

bool NullDevice::doInit(const NativeWindow& window, const DeviceDesc& desc){
  caps_.maxTextureSize = 16384;
  caps_.maxTextureLayers = 2048;
  caps_.maxColorAttachments = MAX_COLOR_ATTACHMENTS;
  caps_.maxVertexAttributes = MAX_VERTEX_ATTRIBUTES;
  caps_.maxUniformBufferSize = 65536;
  caps_.uniformBufferOffsetAlignment = 256;
  caps_.maxAnisotropy = 16;
  caps_.wireframe = true;
  return true;
}

std::unique_ptr<Buffer> NullDevice::createBuffer(const BufferDesc& desc, const void* data){
  return std::make_unique<NullBuffer>(desc);
}

std::unique_ptr<Texture> NullDevice::createTexture(const TextureDesc& desc, const void* data){
  return std::make_unique<NullTexture>(desc);
}

std::unique_ptr<Sampler> NullDevice::createSampler(const SamplerDesc& desc){
  return std::make_unique<NullSampler>(desc);
}

std::unique_ptr<RenderTarget> NullDevice::createRenderTarget(const RenderTargetDesc& desc){
  return std::make_unique<NullRenderTarget>(desc);
}

std::unique_ptr<Shader> NullDevice::createShader(const ShaderDesc& desc){
  return std::make_unique<NullShader>(desc.name);
}

std::unique_ptr<Pipeline> NullDevice::createPipeline(const PipelineDesc& desc){
  return std::make_unique<NullPipeline>(desc);
}
