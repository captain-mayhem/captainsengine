#ifndef CGE_GL_DEVICE_H
#define CGE_GL_DEVICE_H

#include "../renderdevice.h"
#include "GLContext.h"

namespace CGE{
namespace Gfx{

class GLBuffer;
class GLTexture;
class GLPipeline;
class GLRenderTarget;
class GLShader;

//! The OpenGL render device
/*! Owns the window's GL context (WGL, EGL on X11, or EGL with OpenGL ES on
    Android) and loads the plugin's GL functions with glad. On desktop the
    context is OpenGL 3.3 core, or 3.3 compatibility for the old renderers,
    which draw on it until they are ported.
    The resources are GL objects (buffers, textures, sampler objects,
    programs, a vertex array object per pipeline, framebuffers). A state cache
    skips GL calls that wouldn't change anything; with the compatibility
    profile it is reset at every pass, because the old renderers change the
    GL state between the passes. */
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
  void doEndPass() override;
  void doSetPipeline(Pipeline* pipeline) override;
  void doSetVertexBuffer(int slot, Buffer* buffer, size_t offset) override;
  void doSetIndexBuffer(Buffer* buffer, IndexType type, size_t offset) override;
  void doSetUniformBuffer(int slot, Buffer* buffer, size_t offset, size_t size) override;
  void doSetTexture(int slot, Texture* texture, Sampler* sampler) override;
  void doSetViewport(const Viewport& viewport) override;
  void doSetScissor(const Rect& rect) override;
  void doDraw(int vertexCount, int instanceCount, int firstVertex) override;
  void doDrawIndexed(int indexCount, int instanceCount, int firstIndex) override;
  bool doReadPixels(RenderTarget* target, int attachment, const Rect& rect, void* data) override;

private:
  //! false (and an error) if the context can't do what the resources need
  bool checkResources(const char* call);
  std::unique_ptr<GLTexture> makeTexture(const TextureDesc& desc, const void* data);
  //! binds a texture to the unit used for uploads (not one of the slots)
  void bindUploadTexture(GLTexture* texture);
  void setActiveUnit(int unit);
  void bindProgram(GLShader* shader);
  void bindVao(GLPipeline* pipeline);
  //! applies the pipeline's blend, depth and raster state
  void applyRenderState(const PipelineDesc& desc);
  //after uploading: compatibility profile outside passes, back to unit 0
  void endUpload();
  //! sets the attribute pointers and the index buffer of the current pipeline
  bool prepareDraw(bool indexed);
  //! forgets the cached GL state
  void invalidateState();
  //! compatibility profile: saves the old renderers' GL state at the start of a pass
  void saveLegacyState();
  //! and restores it at the end
  void restoreLegacyState();

  std::unique_ptr<GLContext> context_;
  bool hasStencil_;
  //! OpenGL 3.3 or OpenGL ES 3.0, so resources and draws work
  bool resourcesSupported_;
  //! the old renderers draw on this context too
  bool legacy_;
  //! the old renderers' state during a pass
  struct LegacyState{
    int program, framebuffer, vao, arrayBuffer, activeUnit, texture0;
    int viewport[4];
    int scissorBox[4];
    float depthRange[2];
    float clearColor[4];
    float clearDepth;
    int clearStencil, stencilMask;
    unsigned char colorMask[4];
    unsigned char depthMask;
    bool blend, depthTest, cull, scissor, srgb;
    int blendSrcColor, blendDstColor, blendSrcAlpha, blendDstAlpha, blendColorOp, blendAlphaOp;
    int depthFunc, cullFace, frontFace, polygonMode;
  };
  LegacyState legacyState_;

  // the cached GL state (serials, see GLResources.h)
  struct TextureUnit{
    uint64_t texture;
    unsigned target;
    uint64_t sampler;
  };
  struct UniformBinding{
    uint64_t buffer;
    size_t offset;
    size_t size;
  };
  int activeUnit_;
  uint64_t program_;
  uint64_t vao_;
  uint64_t arrayBuffer_;
  TextureUnit units_[MAX_TEXTURE_SLOTS+1];
  UniformBinding uniformBuffers_[MAX_UNIFORM_BUFFERS];
  bool renderStateKnown_;
  BlendState blend_;
  DepthState depth_;
  RasterState raster_;

  // the pass's bindings
  struct VertexBinding{
    GLBuffer* buffer;
    size_t offset;
  };
  unsigned fbo_;
  GLPipeline* pipeline_;
  VertexBinding vertexBuffers_[MAX_VERTEX_BUFFERS];
  GLBuffer* indexBuffer_;
  IndexType indexType_;
  size_t indexOffset_;
};

//! the OpenGL backend's factory (exported by the plugin, or built into the engine)
RenderDevice* createGLRenderDevice();

}
}

#endif
