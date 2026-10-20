#ifndef CGE_GL_DEVICE_H
#define CGE_GL_DEVICE_H

#include "../renderdevice.h"
#include "GLContext.h"

namespace CGE{
namespace Gfx{

//! The OpenGL render device
/*! Owns the window's GL context (WGL, EGL on X11, or EGL with OpenGL ES on
    Android) and loads the plugin's GL functions with glad. On desktop the
    context is OpenGL 3.3 core, or 3.3 compatibility for the old renderers,
    which draw on it until they are ported. The resources, pipelines and draws
    come next (Phase 1b step 3). */
class GLDevice : public RenderDevice{
public:
  GLDevice();
  ~GLDevice();
  const char* getName() const override {return "OpenGL";}
  unsigned long chooseX11Visual(void* display, int screen, const DeviceDesc& desc) override;
  void setVsync(bool vsync) override;

  std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc, const void* data) override;
  std::unique_ptr<Texture> createTexture(const TextureDesc& desc, const void* data) override;
  std::unique_ptr<Sampler> createSampler(const SamplerDesc& desc) override;
  std::unique_ptr<RenderTarget> createRenderTarget(const RenderTargetDesc& desc) override;
  std::unique_ptr<Shader> createShader(const ShaderDesc& desc) override;
  std::unique_ptr<Pipeline> createPipeline(const PipelineDesc& desc) override;
  void updateBuffer(Buffer* buffer, size_t offset, size_t size, const void* data) override;
  void updateTexture(Texture* texture, const TextureRegion& region, const void* data) override;
  void generateMipmaps(Texture* texture) override;

protected:
  bool doInit(const NativeWindow& window, const DeviceDesc& desc) override;
  void doShutdown() override;
  void doEndFrame() override;
  void doBeginPass(const PassDesc& desc, int width, int height) override;
  void doSetPipeline(Pipeline* pipeline) override {}
  void doSetVertexBuffer(int slot, Buffer* buffer, size_t offset) override {}
  void doSetIndexBuffer(Buffer* buffer, IndexType type, size_t offset) override {}
  void doSetUniformBuffer(int slot, Buffer* buffer, size_t offset, size_t size) override {}
  void doSetTexture(int slot, Texture* texture, Sampler* sampler) override {}
  void doSetViewport(const Viewport& viewport) override;
  void doSetScissor(const Rect& rect) override;
  void doDraw(int vertexCount, int instanceCount, int firstVertex) override {}
  void doDrawIndexed(int indexCount, int instanceCount, int firstIndex) override {}

private:
  //! reports a resource kind that this backend can't create yet
  void notYet(const char* what);
  std::unique_ptr<GLContext> context_;
  bool hasStencil_;
};

//! the OpenGL backend's factory (exported by the plugin, or built into the engine)
RenderDevice* createGLRenderDevice();

}
}

#endif
