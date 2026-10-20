#ifndef CGE_GFX_TYPES_H
#define CGE_GFX_TYPES_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

//! Types of the backend-neutral render device interface (renderdevice.h).
/*! No graphics API appears here: the same descriptions work for the OpenGL
    backend and for later Vulkan or Direct3D 12 backends.
    The names avoid X11's macros (None, Always), because the engine's
    headers include Xlib on Linux. */
namespace CGE{
namespace Gfx{

//! binding slots per kind; a backend may support more (see Caps)
const int MAX_VERTEX_BUFFERS = 8;
const int MAX_VERTEX_ATTRIBUTES = 16;
const int MAX_UNIFORM_BUFFERS = 12;
const int MAX_TEXTURE_SLOTS = 16;
const int MAX_COLOR_ATTACHMENTS = 4;

//! the window a device renders into, as native handles
/*! The window backends fill this in; the render backend creates its context
    or surface from it. */
struct NativeWindow{
  enum Kind{
    Headless, //!< no window
    Win32, //!< window: HWND
    X11, //!< display: Display*, window: Window, screen: the screen number
    Android, //!< window: ANativeWindow*
    Emscripten, //!< canvas: CSS selector of the canvas element
  };
  Kind kind = Headless;
  void* display = nullptr;
  uintptr_t window = 0;
  int screen = 0;
  const char* canvas = nullptr;
};

//! settings for creating a device
struct DeviceDesc{
  //! wait for the display's vertical blank when presenting
  bool vsync = true;
  //! backend debug output (e.g. KHR_debug) into the engine's tracing
  bool debug = false;
  //! OpenGL: a compatibility-profile context instead of a core-profile one
  /*! Only for the old renderers, which still use fixed-function GL; it goes
      away with them. Other backends ignore it. */
  bool compatibilityProfile = false;
  //! bits per pixel of the window's colour buffer
  int colorBits = 32;
  //! bits of the window's depth buffer
  int depthBits = 16;
  //! bits of the window's stencil buffer
  int stencilBits = 0;
};

//! what a device can do
struct Caps{
  int maxTextureSize = 0;
  int maxTextureLayers = 0;
  int maxColorAttachments = 0;
  int maxVertexAttributes = 0;
  int maxUniformBufferSize = 0;
  //! uniform buffer offsets in setUniformBuffer must be multiples of this
  int uniformBufferOffsetAlignment = 1;
  float maxAnisotropy = 1;
  //! FillMode::Wireframe works (not on OpenGL ES / WebGL)
  bool wireframe = false;
};

//! how the backend's clip space differs from OpenGL's
/*! RenderDevice::adjustProjection uses this, so engine code never hard-codes
    either convention. */
struct ClipSpace{
  //! depth runs from 0 to 1 (Vulkan, Direct3D) instead of -1 to 1 (OpenGL)
  bool depthZeroToOne = false;
  //! clip-space y points down (Vulkan) instead of up
  bool yDown = false;
  //! row 0 of textures and render targets is the top (Vulkan, Direct3D)
  /*! instead of the bottom (OpenGL) */
  bool originTopLeft = false;
};

struct Rect{
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

struct Viewport{
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;
  float minDepth = 0;
  float maxDepth = 1;
};

struct ClearColor{
  float r = 0;
  float g = 0;
  float b = 0;
  float a = 1;
};

// --- buffers ---

enum class BufferType{
  Vertex,
  Index,
  Uniform,
};

enum class Usage{
  //! written once at creation
  Immutable,
  //! updated now and then
  Dynamic,
  //! rewritten every frame
  Stream,
};

enum class IndexType{
  UInt16,
  UInt32,
};

struct BufferDesc{
  BufferType type = BufferType::Vertex;
  Usage usage = Usage::Immutable;
  //! size in bytes; uniform buffers use the std140 layout
  size_t size = 0;
};

// --- textures and samplers ---

enum class TextureType{
  Texture2D,
  Texture2DArray,
  Cube,
};

enum class Format{
  Undefined,
  R8,
  RG8,
  RGB8,
  RGBA8,
  //! RGBA8 with sRGB-encoded colour channels
  SRGBA8,
  R16F,
  RG16F,
  RGBA16F,
  R32F,
  RG32F,
  RGBA32F,
  Depth16,
  Depth24,
  Depth32F,
  Depth24Stencil8,
};

//! bytes per pixel of a colour or depth format (0 for Undefined)
inline int formatSize(Format fmt){
  switch (fmt){
    case Format::R8: return 1;
    case Format::RG8: return 2;
    case Format::RGB8: return 3;
    case Format::RGBA8: return 4;
    case Format::SRGBA8: return 4;
    case Format::R16F: return 2;
    case Format::RG16F: return 4;
    case Format::RGBA16F: return 8;
    case Format::R32F: return 4;
    case Format::RG32F: return 8;
    case Format::RGBA32F: return 16;
    case Format::Depth16: return 2;
    case Format::Depth24: return 4;
    case Format::Depth32F: return 4;
    case Format::Depth24Stencil8: return 4;
    default: return 0;
  }
}

inline bool isDepthFormat(Format fmt){
  return fmt == Format::Depth16 || fmt == Format::Depth24 || fmt == Format::Depth32F || fmt == Format::Depth24Stencil8;
}

struct TextureDesc{
  TextureType type = TextureType::Texture2D;
  Format format = Format::RGBA8;
  int width = 0;
  int height = 0;
  //! array layers (Texture2DArray); 6 for Cube
  int layers = 1;
  //! 0 means the full mip chain
  int mipLevels = 1;
  //! can be sampled in shaders
  bool sampled = true;
  //! can be attached to a render target
  bool renderTarget = false;
};

//! a part of a texture to update
struct TextureRegion{
  int mipLevel = 0;
  int layer = 0;
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

enum class Filter{
  Nearest,
  Linear,
};

enum class MipFilter{
  //! only mip level 0
  Off,
  Nearest,
  Linear,
};

enum class AddressMode{
  Repeat,
  MirroredRepeat,
  ClampToEdge,
};

struct SamplerDesc{
  Filter minFilter = Filter::Linear;
  Filter magFilter = Filter::Linear;
  MipFilter mipFilter = MipFilter::Off;
  AddressMode addressU = AddressMode::Repeat;
  AddressMode addressV = AddressMode::Repeat;
  AddressMode addressW = AddressMode::Repeat;
  float maxAnisotropy = 1;
};

// --- render targets ---

struct RenderTargetDesc{
  int width = 0;
  int height = 0;
  //! the colour attachments' formats, at most MAX_COLOR_ATTACHMENTS
  std::vector<Format> colorFormats = {Format::RGBA8};
  //! Undefined for no depth attachment
  Format depthFormat = Format::Depth24;
  //! the attachments can be sampled afterwards
  bool sampled = true;
};

// --- shaders ---

//! a name in the shader source and the slot it is bound to
/*! GLSL 3.30 and GLSL ES 3.00 can't declare bindings in the source, so the
    backend binds uniform blocks and samplers by name. Vertex inputs use
    layout(location = n), which both support. */
struct SlotBinding{
  std::string name;
  int slot = 0;
};

struct ShaderDesc{
  //! the source without a #version line (the backend adds it)
  std::string vertexSource;
  std::string fragmentSource;
  //! uniform blocks to uniform buffer slots (setUniformBuffer)
  std::vector<SlotBinding> uniformBlocks;
  //! samplers to texture slots (setTexture)
  std::vector<SlotBinding> textures;
  //! for messages
  std::string name;
};

// --- pipelines ---

enum class VertexFormat{
  Float,
  Float2,
  Float3,
  Float4,
  //! four bytes, normalised to 0..1
  UByte4Norm,
  UByte4,
  Short2,
  Short2Norm,
  Int,
};

struct VertexAttribute{
  //! the shader's layout(location = n)
  int location = 0;
  VertexFormat format = VertexFormat::Float3;
  //! the vertex buffer slot it is read from
  int bufferSlot = 0;
  //! byte offset in the vertex
  int offset = 0;
};

struct VertexBufferLayout{
  //! bytes per vertex (or instance)
  int stride = 0;
  //! advance per instance instead of per vertex
  bool perInstance = false;
};

struct VertexLayout{
  std::vector<VertexAttribute> attributes;
  //! indexed by buffer slot
  std::vector<VertexBufferLayout> buffers;
};

enum class PrimitiveType{
  Points,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip,
};

enum class BlendFactor{
  Zero,
  One,
  SrcColor,
  OneMinusSrcColor,
  DstColor,
  OneMinusDstColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha,
};

enum class BlendOp{
  Add,
  Subtract,
  ReverseSubtract,
  Min,
  Max,
};

enum ColorMask{
  COLOR_R = 1,
  COLOR_G = 2,
  COLOR_B = 4,
  COLOR_A = 8,
  COLOR_ALL = 15,
};

struct BlendState{
  bool enabled = false;
  BlendFactor srcColor = BlendFactor::One;
  BlendFactor dstColor = BlendFactor::Zero;
  BlendOp colorOp = BlendOp::Add;
  BlendFactor srcAlpha = BlendFactor::One;
  BlendFactor dstAlpha = BlendFactor::Zero;
  BlendOp alphaOp = BlendOp::Add;
  //! ColorMask bits
  int writeMask = COLOR_ALL;
};

enum class CompareFunc{
  NeverPass,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  AlwaysPass,
};

struct DepthState{
  bool test = false;
  bool write = false;
  CompareFunc compare = CompareFunc::LessEqual;
};

enum class CullMode{
  Off,
  Front,
  Back,
};

enum class FrontFace{
  CounterClockwise,
  Clockwise,
};

enum class FillMode{
  Solid,
  //! needs Caps::wireframe
  Wireframe,
};

struct RasterState{
  CullMode cull = CullMode::Off;
  FrontFace frontFace = FrontFace::CounterClockwise;
  FillMode fill = FillMode::Solid;
};

class Shader;

//! everything about drawing that is fixed up front
struct PipelineDesc{
  Shader* shader = nullptr;
  VertexLayout vertexLayout;
  PrimitiveType primitive = PrimitiveType::Triangles;
  BlendState blend;
  DepthState depth;
  RasterState raster;
  //! for messages
  std::string name;
};

// --- passes ---

enum class LoadAction{
  //! keep the previous contents
  Load,
  //! clear to the pass's clear value
  Clear,
  //! the contents don't matter (every pixel gets drawn)
  DontCare,
};

class RenderTarget;

struct PassDesc{
  //! nullptr for the window
  RenderTarget* target = nullptr;
  LoadAction colorLoad = LoadAction::Clear;
  LoadAction depthLoad = LoadAction::Clear;
  ClearColor clearColor;
  float clearDepth = 1;
  int clearStencil = 0;
};

}
}

#endif
