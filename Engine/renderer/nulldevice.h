#ifndef CGE_NULL_DEVICE_H
#define CGE_NULL_DEVICE_H

#include "renderdevice.h"

namespace CGE{
namespace Gfx{

//! A render device that draws nothing
/*! For applications without graphics and for tests: it needs no window,
    keeps the resources' descriptions and counts the draw calls. */
class CGE_API NullDevice : public RenderDevice{
public:
  NullDevice();
  const char* getName() const override {return "Null";}
  void setVsync(bool vsync) override {}

  std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc, const void* data) override;
  std::unique_ptr<Texture> createTexture(const TextureDesc& desc, const void* data) override;
  std::unique_ptr<Sampler> createSampler(const SamplerDesc& desc) override;
  std::unique_ptr<RenderTarget> createRenderTarget(const RenderTargetDesc& desc) override;
  std::unique_ptr<Shader> createShader(const ShaderDesc& desc) override;
  std::unique_ptr<Pipeline> createPipeline(const PipelineDesc& desc) override;
  void updateBuffer(Buffer* buffer, size_t offset, size_t size, const void* data) override {}
  void updateTexture(Texture* texture, const TextureRegion& region, const void* data) override {}
  void generateMipmaps(Texture* texture) override {}

  //! frames presented so far
  int getFrameCount() const {return frames_;}
  //! draw calls that reached the backend so far
  int getDrawCount() const {return draws_;}
protected:
  bool doInit(const NativeWindow& window, const DeviceDesc& desc) override;
  void doShutdown() override {}
  void doEndFrame() override {frames_++;}
  void doBeginPass(const PassDesc& desc, int width, int height) override {}
  void doSetPipeline(Pipeline* pipeline) override {}
  void doSetVertexBuffer(int slot, Buffer* buffer, size_t offset) override {}
  void doSetIndexBuffer(Buffer* buffer, IndexType type, size_t offset) override {}
  void doSetUniformBuffer(int slot, Buffer* buffer, size_t offset, size_t size) override {}
  void doSetTexture(int slot, Texture* texture, Sampler* sampler) override {}
  void doSetViewport(const Viewport& viewport) override {}
  void doSetScissor(const Rect& rect) override {}
  void doDraw(int vertexCount, int instanceCount, int firstVertex) override {draws_++;}
  void doDrawIndexed(int indexCount, int instanceCount, int firstIndex) override {draws_++;}
  //! black pixels
  bool doReadPixels(RenderTarget* target, int attachment, const Rect& rect, void* data) override;
private:
  int frames_;
  int draws_;
};

}
}

#endif
