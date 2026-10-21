#ifndef CGE_RENDER_DEVICE_H
#define CGE_RENDER_DEVICE_H

#include <cge_export.h>

#include <memory>
#include <string>

#include "gfxtypes.h"

namespace CGE{

class Matrix;

//! The backend-neutral render device interface
/*! A RenderDevice is implemented once per graphics API (renderer/GL/ for
    OpenGL 3.3 core, OpenGL ES 3.0 and WebGL 2). Backends are chosen by name
    with RenderDevice::create; on desktop they are plugin libraries next to
    the engine library (CGE_GL.dll / libCGE_GL.so).
    The device owns the graphics context of the window: the window backends
    only provide native handles (NativeWindow).
    Drawing follows the model of Vulkan and Direct3D 12: immutable resources,
    pipeline state objects instead of state toggles, resources bound to
    numbered slots, and draw calls inside passes inside frames. */
namespace Gfx{

//! base of all device resources; destroyed by deleting them
class CGE_API Resource{
public:
  virtual ~Resource();
  Resource(const Resource&) = delete;
  Resource& operator=(const Resource&) = delete;
protected:
  Resource() = default;
};

class CGE_API Buffer : public Resource{
public:
  const BufferDesc& getDesc() const {return desc_;}
protected:
  explicit Buffer(const BufferDesc& desc) : desc_(desc) {}
  BufferDesc desc_;
};

class CGE_API Texture : public Resource{
public:
  const TextureDesc& getDesc() const {return desc_;}
protected:
  explicit Texture(const TextureDesc& desc) : desc_(desc) {}
  TextureDesc desc_;
};

class CGE_API Sampler : public Resource{
public:
  const SamplerDesc& getDesc() const {return desc_;}
protected:
  explicit Sampler(const SamplerDesc& desc) : desc_(desc) {}
  SamplerDesc desc_;
};

class CGE_API Shader : public Resource{
public:
  const std::string& getName() const {return name_;}
protected:
  explicit Shader(const std::string& name) : name_(name) {}
  std::string name_;
};

class CGE_API Pipeline : public Resource{
public:
  const PipelineDesc& getDesc() const {return desc_;}
protected:
  explicit Pipeline(const PipelineDesc& desc) : desc_(desc) {}
  PipelineDesc desc_;
};

class CGE_API RenderTarget : public Resource{
public:
  const RenderTargetDesc& getDesc() const {return desc_;}
  //! colour attachment idx, owned by the render target
  virtual Texture* getColorTexture(int idx) const = 0;
  //! the depth attachment, or nullptr
  virtual Texture* getDepthTexture() const = 0;
protected:
  explicit RenderTarget(const RenderTargetDesc& desc) : desc_(desc) {}
  RenderTargetDesc desc_;
};

class CGE_API RenderDevice{
public:
  //! creates the device of a backend
  /*! "OpenGL" (a plugin library on desktop) or "Null" (draws nothing, for
      headless use and tests). Returns nullptr if the backend is unknown or
      its library can't be loaded. The device still has to be initialised. */
  static std::unique_ptr<RenderDevice> create(const std::string& backend);

  virtual ~RenderDevice();
  RenderDevice(const RenderDevice&) = delete;
  RenderDevice& operator=(const RenderDevice&) = delete;

  //! the backend's name as given to create
  virtual const char* getName() const = 0;

  //! X11 only: the id of the visual the window has to be created with
  /*! Called before init, before the window exists, with the desc that init
      gets. 0 means the screen's default visual. */
  virtual unsigned long chooseX11Visual(void* display, int screen, const DeviceDesc& desc) {return 0;}

  //! creates the graphics context or surface for the window
  bool init(const NativeWindow& window, const DeviceDesc& desc);
  //! destroys the context; all resources must be deleted before
  void shutdown();
  bool isInitialized() const {return state_ != State::Uninitialized;}

  //! the window was resized to width x height pixels
  void resize(int width, int height);
  //! the size of the window's drawable area in pixels
  int getWidth() const {return width_;}
  int getHeight() const {return height_;}
  //! turns waiting for the vertical blank on or off
  virtual void setVsync(bool vsync) = 0;

  const Caps& getCaps() const {return caps_;}
  const ClipSpace& getClipSpace() const {return clipSpace_;}
  //! converts a projection matrix made for OpenGL's clip space to this device's
  Matrix adjustProjection(const Matrix& glProjection) const;
  //! an orthographic projection for this device's clip space
  Matrix ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane) const;
  //! a perspective projection for this device's clip space
  /*! fovy is the vertical field of view in degrees */
  Matrix perspective(float fovy, float aspect, float nearPlane, float farPlane) const;

  // --- resources, created and updated outside or inside frames ---

  //! a buffer of desc.size bytes, filled with data if it isn't nullptr
  virtual std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc, const void* data = nullptr) = 0;
  //! a texture whose mip level 0 is filled with data (tightly packed, all layers) if it isn't nullptr
  virtual std::unique_ptr<Texture> createTexture(const TextureDesc& desc, const void* data = nullptr) = 0;
  virtual std::unique_ptr<Sampler> createSampler(const SamplerDesc& desc) = 0;
  virtual std::unique_ptr<RenderTarget> createRenderTarget(const RenderTargetDesc& desc) = 0;
  //! compiles and links a shader; nullptr (and an error message) if that fails
  virtual std::unique_ptr<Shader> createShader(const ShaderDesc& desc) = 0;
  virtual std::unique_ptr<Pipeline> createPipeline(const PipelineDesc& desc) = 0;
  //! writes size bytes at offset; not for Immutable buffers
  virtual void updateBuffer(Buffer* buffer, size_t offset, size_t size, const void* data) = 0;
  //! writes a region of a texture (tightly packed)
  virtual void updateTexture(Texture* texture, const TextureRegion& region, const void* data) = 0;
  //! fills mip levels 1 and up from level 0
  virtual void generateMipmaps(Texture* texture) = 0;

  // --- frames and passes ---

  //! starts a frame; false if the device can't render now
  bool beginFrame();
  //! ends the frame and presents it in the window
  void endFrame();
  //! starts a pass into the window (desc.target == nullptr) or a render target
  /*! Sets the viewport and the scissor rectangle to the whole target. */
  bool beginPass(const PassDesc& desc);
  void endPass();
  bool isInPass() const {return state_ == State::InPass;}

  // --- inside a pass ---

  void setPipeline(Pipeline* pipeline);
  void setVertexBuffer(int slot, Buffer* buffer, size_t offset = 0);
  void setIndexBuffer(Buffer* buffer, IndexType type, size_t offset = 0);
  //! binds size bytes from offset (0: the rest of the buffer) to a uniform slot
  void setUniformBuffer(int slot, Buffer* buffer, size_t offset = 0, size_t size = 0);
  void setTexture(int slot, Texture* texture, Sampler* sampler);
  void setViewport(const Viewport& viewport);
  void setScissor(const Rect& rect);
  void draw(int vertexCount, int instanceCount = 1, int firstVertex = 0);
  void drawIndexed(int indexCount, int instanceCount = 1, int firstIndex = 0);

  // --- reading back ---

  //! reads pixels of the window (target == nullptr) or of a render target's colour attachment
  /*! data gets rect.width*rect.height RGBA8 pixels, rows from the bottom up
      like rect. The attachment's format must be RGBA8 or SRGBA8. The window
      is read from the frame being drawn, so call it before endFrame.
      Slow: for tests and screenshots. */
  bool readPixels(RenderTarget* target, int attachment, const Rect& rect, void* data);

protected:
  RenderDevice();

  // implemented by the backends; the public functions check the state first

  //! creates the context and fills in caps_, clipSpace_, width_ and height_
  virtual bool doInit(const NativeWindow& window, const DeviceDesc& desc) = 0;
  virtual void doShutdown() = 0;
  virtual void doResize(int width, int height) {}
  virtual bool doBeginFrame() {return true;}
  virtual void doEndFrame() = 0;
  //! the pass's width and height are passed in
  virtual void doBeginPass(const PassDesc& desc, int width, int height) = 0;
  virtual void doEndPass() {}
  virtual void doSetPipeline(Pipeline* pipeline) = 0;
  virtual void doSetVertexBuffer(int slot, Buffer* buffer, size_t offset) = 0;
  virtual void doSetIndexBuffer(Buffer* buffer, IndexType type, size_t offset) = 0;
  virtual void doSetUniformBuffer(int slot, Buffer* buffer, size_t offset, size_t size) = 0;
  virtual void doSetTexture(int slot, Texture* texture, Sampler* sampler) = 0;
  virtual void doSetViewport(const Viewport& viewport) = 0;
  virtual void doSetScissor(const Rect& rect) = 0;
  virtual void doDraw(int vertexCount, int instanceCount, int firstVertex) = 0;
  virtual void doDrawIndexed(int indexCount, int instanceCount, int firstIndex) = 0;
  virtual bool doReadPixels(RenderTarget* target, int attachment, const Rect& rect, void* data) = 0;

  Caps caps_;
  ClipSpace clipSpace_;
  int width_;
  int height_;

private:
  enum class State{
    Uninitialized,
    Idle,
    InFrame,
    InPass,
  };
  //! reports an error unless the device is in the state expected
  bool checkState(State expected, const char* call) const;
  State state_;
  Pipeline* pipeline_;
  bool indexBufferSet_;
};

}
}

#endif
