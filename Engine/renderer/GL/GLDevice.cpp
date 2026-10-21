#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <cstring>

#include "GLDevice.h"
#include "GLResources.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL(CGE_Renderer_GL);

RenderDevice* CGE::Gfx::createGLRenderDevice(){
  return new GLDevice();
}

namespace{

//! textures are uploaded on the unit after the slots, so the slots' bindings stay
const int UPLOAD_UNIT = MAX_TEXTURE_SLOTS;

const char* SHADER_HEADER =
#if defined(CGE_GLES)
  "#version 300 es\n"
  "precision highp float;\n"
  "precision highp int;\n"
  "precision highp sampler2DArray;\n"
#else
  "#version 330 core\n"
#endif
  //the source's own line numbers in the compiler's messages
  "#line 1\n";

#if !defined(CGE_GLES)
GLADapiproc loadGLFunction(void* context, const char* name){
  return (GLADapiproc)((GLContext*)context)->getProcAddress(name);
}

//! KHR_debug output into the tracing
void GLAD_API_PTR debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user){
  TR_USE(CGE_Renderer_GL);
  if (type == GL_DEBUG_TYPE_ERROR || severity == GL_DEBUG_SEVERITY_HIGH)
    TR_ERROR("GL %u: %s", id, message);
  else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
    TR_WARN("GL %u: %s", id, message);
  else
    TR_INFO("GL %u: %s", id, message);
}
#endif

GLenum toGL(BlendFactor factor){
  switch (factor){
    case BlendFactor::Zero: return GL_ZERO;
    case BlendFactor::One: return GL_ONE;
    case BlendFactor::SrcColor: return GL_SRC_COLOR;
    case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DstColor: return GL_DST_COLOR;
    case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
    case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DstAlpha: return GL_DST_ALPHA;
    case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
  }
  return GL_ONE;
}

GLenum toGL(BlendOp op){
  switch (op){
    case BlendOp::Add: return GL_FUNC_ADD;
    case BlendOp::Subtract: return GL_FUNC_SUBTRACT;
    case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    case BlendOp::Min: return GL_MIN;
    case BlendOp::Max: return GL_MAX;
  }
  return GL_FUNC_ADD;
}

GLenum toGL(CompareFunc func){
  switch (func){
    case CompareFunc::NeverPass: return GL_NEVER;
    case CompareFunc::Less: return GL_LESS;
    case CompareFunc::Equal: return GL_EQUAL;
    case CompareFunc::LessEqual: return GL_LEQUAL;
    case CompareFunc::Greater: return GL_GREATER;
    case CompareFunc::NotEqual: return GL_NOTEQUAL;
    case CompareFunc::GreaterEqual: return GL_GEQUAL;
    case CompareFunc::AlwaysPass: return GL_ALWAYS;
  }
  return GL_LEQUAL;
}

GLenum toGL(AddressMode mode){
  switch (mode){
    case AddressMode::Repeat: return GL_REPEAT;
    case AddressMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
    case AddressMode::ClampToEdge: return GL_CLAMP_TO_EDGE;
  }
  return GL_REPEAT;
}

GLenum toGLMinFilter(Filter filter, MipFilter mip){
  bool linear = filter == Filter::Linear;
  switch (mip){
    case MipFilter::Nearest: return linear ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
    case MipFilter::Linear: return linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
    default: return linear ? GL_LINEAR : GL_NEAREST;
  }
}

//! components, GL type, normalised, integer input
struct GLVertexFormat{
  GLint size;
  GLenum type;
  GLboolean normalized;
  bool integer;
};

GLVertexFormat toGL(VertexFormat format){
  switch (format){
    case VertexFormat::Float: return {1, GL_FLOAT, GL_FALSE, false};
    case VertexFormat::Float2: return {2, GL_FLOAT, GL_FALSE, false};
    case VertexFormat::Float3: return {3, GL_FLOAT, GL_FALSE, false};
    case VertexFormat::Float4: return {4, GL_FLOAT, GL_FALSE, false};
    case VertexFormat::UByte4Norm: return {4, GL_UNSIGNED_BYTE, GL_TRUE, false};
    case VertexFormat::UByte4: return {4, GL_UNSIGNED_BYTE, GL_FALSE, true};
    case VertexFormat::Short2: return {2, GL_SHORT, GL_FALSE, true};
    case VertexFormat::Short2Norm: return {2, GL_SHORT, GL_TRUE, false};
    case VertexFormat::Int: return {1, GL_INT, GL_FALSE, true};
  }
  return {3, GL_FLOAT, GL_FALSE, false};
}

GLenum toGLTarget(TextureType type){
  switch (type){
    case TextureType::Texture2DArray: return GL_TEXTURE_2D_ARRAY;
    case TextureType::Cube: return GL_TEXTURE_CUBE_MAP;
    default: return GL_TEXTURE_2D;
  }
}

void enable(GLenum cap, bool on){
  if (on)
    glEnable(cap);
  else
    glDisable(cap);
}

int fullMipChain(int width, int height){
  int levels = 1;
  int size = (std::max)(width, height);
  while (size > 1){
    size /= 2;
    levels++;
  }
  return levels;
}

//! the compile or link log of a shader or program
std::string infoLog(GLuint object, bool program){
  GLint length = 0;
  if (program)
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
  else
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
  if (length <= 0)
    return "";
  std::string log(length, '\0');
  if (program)
    glGetProgramInfoLog(object, length, nullptr, &log[0]);
  else
    glGetShaderInfoLog(object, length, nullptr, &log[0]);
  log.resize(strlen(log.c_str()));
  return log;
}

//! compiles one stage with the version header; 0 (and an error) if that fails
GLuint compileStage(GLenum stage, const std::string& source, const std::string& name){
  TR_USE(CGE_Renderer_GL);
  GLuint shader = glCreateShader(stage);
  const GLchar* sources[] = {SHADER_HEADER, source.c_str()};
  glShaderSource(shader, 2, sources, nullptr);
  glCompileShader(shader);
  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok){
    TR_ERROR("The %s shader of %s doesn't compile:\n%s", stage == GL_VERTEX_SHADER ? "vertex" : "fragment", name.c_str(), infoLog(shader, false).c_str());
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

}

GLDevice::GLDevice(){
  context_ = GLContext::create();
  hasStencil_ = false;
  resourcesSupported_ = false;
  legacy_ = false;
  legacyState_ = LegacyState();
  fbo_ = 0;
  pipeline_ = nullptr;
  indexBuffer_ = nullptr;
  indexType_ = IndexType::UInt16;
  indexOffset_ = 0;
  for (VertexBinding& binding : vertexBuffers_)
    binding = {nullptr, 0};
  invalidateState();
}

GLDevice::~GLDevice(){
  shutdown();
}

unsigned long GLDevice::chooseX11Visual(void* display, int screen, const DeviceDesc& desc){
  return context_->chooseX11Visual(display, screen, desc);
}

bool GLDevice::doInit(const NativeWindow& window, const DeviceDesc& desc){
  TR_USE(CGE_Renderer_GL);
  if (!context_->init(window, desc, width_, height_)){
    context_->shutdown();
    return false;
  }
#if defined(CGE_GLES)
  TR_INFO("OpenGL %s on %s", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_RENDERER));
  //an OpenGL ES 2.0 context doesn't know GL_MAJOR_VERSION and leaves the value
  GLint major = 2;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  while (glGetError() != GL_NO_ERROR){}
  resourcesSupported_ = major >= 3;
  if (!resourcesSupported_)
    TR_WARN("Not an OpenGL ES 3.0 context: only the old renderers work on it");
#else
  int version = gladLoadGLUserPtr(loadGLFunction, context_.get());
  if (!version){
    TR_ERROR("Can't load the OpenGL functions");
    context_->shutdown();
    return false;
  }
  const char* profile = "";
  if (GLAD_VERSION_MAJOR(version)*10+GLAD_VERSION_MINOR(version) >= 32){
    GLint mask = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
    profile = (mask & GL_CONTEXT_CORE_PROFILE_BIT) ? ", core profile" : ", compatibility profile";
  }
  TR_INFO("OpenGL %s on %s%s", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_RENDERER), profile);
  resourcesSupported_ = GLAD_GL_VERSION_3_3 != 0;
  if (!resourcesSupported_)
    TR_WARN("Not an OpenGL 3.3 context: only the old renderers work on it");
  if (desc.debug && GLAD_GL_KHR_debug){
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugMessage, nullptr);
    //drivers report every buffer allocation and the like as notifications
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    //createShader traces the compile logs itself (Mesa reports them again)
    glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
  }
#endif
  context_->setSwapInterval(desc.vsync ? 1 : 0);
  hasStencil_ = desc.stencilBits > 0;
  legacy_ = desc.compatibilityProfile;

  GLint value = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
  caps_.maxTextureSize = value;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
  caps_.maxVertexAttributes = value;
  caps_.maxTextureLayers = 1;
  caps_.maxColorAttachments = 1;
  caps_.maxUniformBufferSize = 0;
  caps_.uniformBufferOffsetAlignment = 256;
  if (resourcesSupported_){
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &value);
    caps_.maxTextureLayers = value;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
    caps_.maxColorAttachments = value;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
    caps_.maxUniformBufferSize = value;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    caps_.uniformBufferOffsetAlignment = value;
  }
  caps_.maxAnisotropy = 1;
#if defined(CGE_GLES)
  //anisotropic filtering and wireframe come with the Android port (Phase 1c)
  caps_.wireframe = false;
#else
  if (GLAD_GL_ARB_texture_filter_anisotropic || GLAD_GL_EXT_texture_filter_anisotropic){
    GLfloat anisotropy = 1;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
    caps_.maxAnisotropy = anisotropy;
  }
  caps_.wireframe = true;
#endif
  //OpenGL's conventions are the reference, so nothing differs
  clipSpace_ = ClipSpace();

  //uploads and read-backs are tightly packed (the old renderers also use 1)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  invalidateState();
  return true;
}

void GLDevice::doShutdown(){
  context_->shutdown();
  resourcesSupported_ = false;
}

void GLDevice::setVsync(bool vsync){
  if (isInitialized())
    context_->setSwapInterval(vsync ? 1 : 0);
}

void GLDevice::doEndFrame(){
  context_->swapBuffers();
}

bool GLDevice::checkResources(const char* call){
  TR_USE(CGE_Renderer_GL);
  if (resourcesSupported_)
    return true;
  TR_ERROR("%s: the GL backend needs OpenGL 3.3 or OpenGL ES 3.0", call);
  return false;
}

// --- the state cache ---

void GLDevice::invalidateState(){
  activeUnit_ = -1;
  program_ = GL_UNKNOWN;
  vao_ = GL_UNKNOWN;
  arrayBuffer_ = GL_UNKNOWN;
  for (TextureUnit& unit : units_)
    unit = {GL_UNKNOWN, 0, GL_UNKNOWN};
  for (UniformBinding& binding : uniformBuffers_)
    binding = {GL_UNKNOWN, 0, 0};
  renderStateKnown_ = false;
}

void GLDevice::setActiveUnit(int unit){
  if (activeUnit_ == unit)
    return;
  glActiveTexture(GL_TEXTURE0+unit);
  activeUnit_ = unit;
}

void GLDevice::bindUploadTexture(GLTexture* texture){
  TextureUnit& unit = units_[UPLOAD_UNIT];
  setActiveUnit(UPLOAD_UNIT);
  if (unit.texture == texture->getSerial())
    return;
  glBindTexture(texture->getTarget(), texture->getId());
  unit.texture = texture->getSerial();
  unit.target = texture->getTarget();
}

void GLDevice::bindProgram(GLShader* shader){
  uint64_t serial = shader ? shader->getSerial() : 0;
  if (program_ == serial)
    return;
  glUseProgram(shader ? shader->getProgram() : 0);
  program_ = serial;
}

void GLDevice::bindVao(GLPipeline* pipeline){
  uint64_t serial = pipeline ? pipeline->getSerial() : 0;
  if (vao_ == serial)
    return;
  glBindVertexArray(pipeline ? pipeline->getVao() : 0);
  vao_ = serial;
}

void GLDevice::applyRenderState(const PipelineDesc& desc){
  //with an unknown state everything is set, including the blend factors of a pipeline without blending
  bool all = !renderStateKnown_;
  const BlendState& blend = desc.blend;
  if (all || blend.enabled != blend_.enabled){
    if (blend.enabled)
      glEnable(GL_BLEND);
    else
      glDisable(GL_BLEND);
    blend_.enabled = blend.enabled;
  }
  if (all || (blend.enabled && (blend.srcColor != blend_.srcColor || blend.dstColor != blend_.dstColor || blend.srcAlpha != blend_.srcAlpha || blend.dstAlpha != blend_.dstAlpha))){
    glBlendFuncSeparate(toGL(blend.srcColor), toGL(blend.dstColor), toGL(blend.srcAlpha), toGL(blend.dstAlpha));
    blend_.srcColor = blend.srcColor;
    blend_.dstColor = blend.dstColor;
    blend_.srcAlpha = blend.srcAlpha;
    blend_.dstAlpha = blend.dstAlpha;
  }
  if (all || (blend.enabled && (blend.colorOp != blend_.colorOp || blend.alphaOp != blend_.alphaOp))){
    glBlendEquationSeparate(toGL(blend.colorOp), toGL(blend.alphaOp));
    blend_.colorOp = blend.colorOp;
    blend_.alphaOp = blend.alphaOp;
  }
  if (all || blend.writeMask != blend_.writeMask){
    glColorMask((blend.writeMask & COLOR_R) != 0, (blend.writeMask & COLOR_G) != 0, (blend.writeMask & COLOR_B) != 0, (blend.writeMask & COLOR_A) != 0);
    blend_.writeMask = blend.writeMask;
  }

  const DepthState& depth = desc.depth;
  if (all || depth.test != depth_.test){
    if (depth.test)
      glEnable(GL_DEPTH_TEST);
    else
      glDisable(GL_DEPTH_TEST);
  }
  if (all || depth.write != depth_.write)
    glDepthMask(depth.write ? GL_TRUE : GL_FALSE);
  if (all || depth.compare != depth_.compare)
    glDepthFunc(toGL(depth.compare));
  depth_ = depth;

  const RasterState& raster = desc.raster;
  if (all || raster.cull != raster_.cull){
    if (raster.cull == CullMode::Off)
      glDisable(GL_CULL_FACE);
    else{
      glEnable(GL_CULL_FACE);
      glCullFace(raster.cull == CullMode::Front ? GL_FRONT : GL_BACK);
    }
  }
  if (all || raster.frontFace != raster_.frontFace)
    glFrontFace(raster.frontFace == FrontFace::Clockwise ? GL_CW : GL_CCW);
#if !defined(CGE_GLES)
  if (all || raster.fill != raster_.fill)
    glPolygonMode(GL_FRONT_AND_BACK, raster.fill == FillMode::Wireframe ? GL_LINE : GL_FILL);
#endif
  raster_ = raster;
  renderStateKnown_ = true;
}

void GLDevice::endUpload(){
  //the old renderers enable fixed-function texturing on the active unit
  if (legacy_ && !isInPass())
    setActiveUnit(0);
}

void GLDevice::saveLegacyState(){
  LegacyState& s = legacyState_;
  glGetIntegerv(GL_CURRENT_PROGRAM, &s.program);
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s.framebuffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &s.vao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &s.arrayBuffer);
  glGetIntegerv(GL_ACTIVE_TEXTURE, &s.activeUnit);
  glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &s.texture0);
  glGetIntegerv(GL_VIEWPORT, s.viewport);
  glGetIntegerv(GL_SCISSOR_BOX, s.scissorBox);
  glGetFloatv(GL_DEPTH_RANGE, s.depthRange);
  glGetFloatv(GL_COLOR_CLEAR_VALUE, s.clearColor);
  glGetFloatv(GL_DEPTH_CLEAR_VALUE, &s.clearDepth);
  glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &s.clearStencil);
  glGetIntegerv(GL_STENCIL_WRITEMASK, &s.stencilMask);
  glGetBooleanv(GL_COLOR_WRITEMASK, s.colorMask);
  glGetBooleanv(GL_DEPTH_WRITEMASK, &s.depthMask);
  s.blend = glIsEnabled(GL_BLEND) != GL_FALSE;
  s.depthTest = glIsEnabled(GL_DEPTH_TEST) != GL_FALSE;
  s.cull = glIsEnabled(GL_CULL_FACE) != GL_FALSE;
  s.scissor = glIsEnabled(GL_SCISSOR_TEST) != GL_FALSE;
  glGetIntegerv(GL_BLEND_SRC_RGB, &s.blendSrcColor);
  glGetIntegerv(GL_BLEND_DST_RGB, &s.blendDstColor);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &s.blendSrcAlpha);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &s.blendDstAlpha);
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &s.blendColorOp);
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &s.blendAlphaOp);
  glGetIntegerv(GL_DEPTH_FUNC, &s.depthFunc);
  glGetIntegerv(GL_CULL_FACE_MODE, &s.cullFace);
  glGetIntegerv(GL_FRONT_FACE, &s.frontFace);
#if !defined(CGE_GLES)
  s.srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB) != GL_FALSE;
  GLint polygonMode[2] = {GL_FILL, GL_FILL};
  glGetIntegerv(GL_POLYGON_MODE, polygonMode);
  s.polygonMode = polygonMode[0];
#endif
}

void GLDevice::restoreLegacyState(){
  const LegacyState& s = legacyState_;
  glBindFramebuffer(GL_FRAMEBUFFER, s.framebuffer);
  glBindVertexArray(s.vao);
  glUseProgram(s.program);
  glBindBuffer(GL_ARRAY_BUFFER, s.arrayBuffer);
  //sampler objects would override the old renderers' texture parameters
  for (int i = 0; i <= MAX_TEXTURE_SLOTS; i++){
    if (units_[i].sampler != 0 && units_[i].sampler != GL_UNKNOWN)
      glBindSampler(i, 0);
  }
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, s.texture0);
  glActiveTexture(s.activeUnit);
  glViewport(s.viewport[0], s.viewport[1], s.viewport[2], s.viewport[3]);
  glScissor(s.scissorBox[0], s.scissorBox[1], s.scissorBox[2], s.scissorBox[3]);
  glClearColor(s.clearColor[0], s.clearColor[1], s.clearColor[2], s.clearColor[3]);
  glClearStencil(s.clearStencil);
  glStencilMask(s.stencilMask);
  glColorMask(s.colorMask[0], s.colorMask[1], s.colorMask[2], s.colorMask[3]);
  glDepthMask(s.depthMask);
  enable(GL_BLEND, s.blend);
  enable(GL_DEPTH_TEST, s.depthTest);
  enable(GL_CULL_FACE, s.cull);
  enable(GL_SCISSOR_TEST, s.scissor);
  glBlendFuncSeparate(s.blendSrcColor, s.blendDstColor, s.blendSrcAlpha, s.blendDstAlpha);
  glBlendEquationSeparate(s.blendColorOp, s.blendAlphaOp);
  glDepthFunc(s.depthFunc);
  glCullFace(s.cullFace);
  glFrontFace(s.frontFace);
#if defined(CGE_GLES)
  glDepthRangef(s.depthRange[0], s.depthRange[1]);
  glClearDepthf(s.clearDepth);
#else
  glDepthRange(s.depthRange[0], s.depthRange[1]);
  glClearDepth(s.clearDepth);
  enable(GL_FRAMEBUFFER_SRGB, s.srgb);
  glPolygonMode(GL_FRONT_AND_BACK, s.polygonMode);
#endif
  invalidateState();
}

// --- passes and draws ---

void GLDevice::doBeginPass(const PassDesc& desc, int width, int height){
  TR_USE(CGE_Renderer_GL);
  //the old renderers changed the GL state since the last pass
  if (legacy_){
    invalidateState();
    if (resourcesSupported_)
      saveLegacyState();
  }
  pipeline_ = nullptr;
  indexBuffer_ = nullptr;
  for (VertexBinding& binding : vertexBuffers_)
    binding = {nullptr, 0};
  bool stencil = hasStencil_;
  if (resourcesSupported_){
    GLRenderTarget* target = (GLRenderTarget*)desc.target;
    fbo_ = target ? target->getFbo() : 0;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    if (target)
      stencil = target->hasStencil();
#if !defined(CGE_GLES)
    //sRGB attachments convert on writing like on OpenGL ES; the window
    //stays as it is, even if the driver gave it an sRGB-capable format
    if (target)
      glEnable(GL_FRAMEBUFFER_SRGB);
    else
      glDisable(GL_FRAMEBUFFER_SRGB);
    glDepthRange(0, 1);
#else
    glDepthRangef(0, 1);
#endif
  }
  else if (desc.target)
    TR_ERROR("Render targets need OpenGL 3.3 or OpenGL ES 3.0; drawing into the window");
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  glEnable(GL_SCISSOR_TEST);
  GLbitfield mask = 0;
  //clearing obeys the write masks
  if (desc.colorLoad == LoadAction::Clear){
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    blend_.writeMask = COLOR_ALL;
    glClearColor(desc.clearColor.r, desc.clearColor.g, desc.clearColor.b, desc.clearColor.a);
    mask |= GL_COLOR_BUFFER_BIT;
  }
  if (desc.depthLoad == LoadAction::Clear){
    glDepthMask(GL_TRUE);
    depth_.write = true;
#if defined(CGE_GLES)
    glClearDepthf(desc.clearDepth);
#else
    glClearDepth(desc.clearDepth);
#endif
    mask |= GL_DEPTH_BUFFER_BIT;
    if (stencil){
      glStencilMask(0xFF);
      glClearStencil(desc.clearStencil);
      mask |= GL_STENCIL_BUFFER_BIT;
    }
  }
  if (mask)
    glClear(mask);
}

void GLDevice::doEndPass(){
  pipeline_ = nullptr;
  indexBuffer_ = nullptr;
  if (legacy_ && resourcesSupported_)
    restoreLegacyState();
}

void GLDevice::doSetPipeline(Pipeline* pipeline){
  if (!resourcesSupported_)
    return;
  GLPipeline* glPipeline = (GLPipeline*)pipeline;
  pipeline_ = glPipeline;
  bindProgram(glPipeline->getShader());
  bindVao(glPipeline);
  applyRenderState(glPipeline->getDesc());
}

void GLDevice::doSetVertexBuffer(int slot, Buffer* buffer, size_t offset){
  vertexBuffers_[slot] = {(GLBuffer*)buffer, offset};
}

void GLDevice::doSetIndexBuffer(Buffer* buffer, IndexType type, size_t offset){
  indexBuffer_ = (GLBuffer*)buffer;
  indexType_ = type;
  indexOffset_ = offset;
}

void GLDevice::doSetUniformBuffer(int slot, Buffer* buffer, size_t offset, size_t size){
  if (!resourcesSupported_)
    return;
  GLBuffer* glBuffer = (GLBuffer*)buffer;
  UniformBinding& binding = uniformBuffers_[slot];
  uint64_t serial = glBuffer ? glBuffer->getSerial() : 0;
  if (binding.buffer == serial && binding.offset == offset && binding.size == size)
    return;
  if (glBuffer)
    glBindBufferRange(GL_UNIFORM_BUFFER, slot, glBuffer->getId(), offset, size);
  else
    glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0);
  binding = {serial, offset, size};
}

void GLDevice::doSetTexture(int slot, Texture* texture, Sampler* sampler){
  if (!resourcesSupported_)
    return;
  GLTexture* glTexture = (GLTexture*)texture;
  TextureUnit& unit = units_[slot];
  uint64_t serial = glTexture ? glTexture->getSerial() : 0;
  if (unit.texture != serial){
    setActiveUnit(slot);
    //a unit has one binding per target; don't leave another target's texture there
    if (unit.texture != 0 && unit.target != 0 && (!glTexture || unit.target != glTexture->getTarget()))
      glBindTexture(unit.target, 0);
    if (glTexture)
      glBindTexture(glTexture->getTarget(), glTexture->getId());
    unit.texture = serial;
    unit.target = glTexture ? glTexture->getTarget() : 0;
  }
  GLSampler* glSampler = glTexture ? (GLSampler*)sampler : nullptr;
  uint64_t samplerSerial = glSampler ? glSampler->getSerial() : 0;
  if (unit.sampler != samplerSerial){
    glBindSampler(slot, glSampler ? glSampler->getId() : 0);
    unit.sampler = samplerSerial;
  }
}

void GLDevice::doSetViewport(const Viewport& viewport){
  glViewport((GLint)viewport.x, (GLint)viewport.y, (GLsizei)viewport.width, (GLsizei)viewport.height);
#if defined(CGE_GLES)
  glDepthRangef(viewport.minDepth, viewport.maxDepth);
#else
  glDepthRange(viewport.minDepth, viewport.maxDepth);
#endif
}

void GLDevice::doSetScissor(const Rect& rect){
  glScissor(rect.x, rect.y, rect.width, rect.height);
}

bool GLDevice::prepareDraw(bool indexed){
  TR_USE(CGE_Renderer_GL);
  if (!resourcesSupported_ || !pipeline_)
    return false;
  GLPipeline* pipeline = pipeline_;
  bindVao(pipeline);
  const VertexLayout& layout = pipeline->getDesc().vertexLayout;
  for (size_t i = 0; i < layout.attributes.size(); i++){
    const VertexAttribute& attr = layout.attributes[i];
    const VertexBinding& binding = vertexBuffers_[attr.bufferSlot];
    if (!binding.buffer){
      TR_ERROR("%s: no vertex buffer in slot %d", pipeline->getDesc().name.c_str(), attr.bufferSlot);
      return false;
    }
    size_t offset = binding.offset+attr.offset;
    GLPipeline::AttributeSource& source = pipeline->attributeSources[i];
    if (source.buffer == binding.buffer->getSerial() && source.offset == offset)
      continue;
    if (arrayBuffer_ != binding.buffer->getSerial()){
      glBindBuffer(GL_ARRAY_BUFFER, binding.buffer->getId());
      arrayBuffer_ = binding.buffer->getSerial();
    }
    GLsizei stride = attr.bufferSlot < (int)layout.buffers.size() ? layout.buffers[attr.bufferSlot].stride : 0;
    GLVertexFormat format = toGL(attr.format);
    if (format.integer)
      glVertexAttribIPointer(attr.location, format.size, format.type, stride, (const void*)offset);
    else
      glVertexAttribPointer(attr.location, format.size, format.type, format.normalized, stride, (const void*)offset);
    source.buffer = binding.buffer->getSerial();
    source.offset = offset;
  }
  if (indexed && pipeline->indexBuffer != indexBuffer_->getSerial()){
    //part of the VAO's state
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_->getId());
    pipeline->indexBuffer = indexBuffer_->getSerial();
  }
  return true;
}

void GLDevice::doDraw(int vertexCount, int instanceCount, int firstVertex){
  if (!prepareDraw(false))
    return;
  if (instanceCount == 1)
    glDrawArrays(pipeline_->getMode(), firstVertex, vertexCount);
  else
    glDrawArraysInstanced(pipeline_->getMode(), firstVertex, vertexCount, instanceCount);
}

void GLDevice::doDrawIndexed(int indexCount, int instanceCount, int firstIndex){
  if (!prepareDraw(true))
    return;
  GLenum type = indexType_ == IndexType::UInt32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
  size_t offset = indexOffset_+(size_t)firstIndex*(indexType_ == IndexType::UInt32 ? 4 : 2);
  if (instanceCount == 1)
    glDrawElements(pipeline_->getMode(), indexCount, type, (const void*)offset);
  else
    glDrawElementsInstanced(pipeline_->getMode(), indexCount, type, (const void*)offset, instanceCount);
}

bool GLDevice::doReadPixels(RenderTarget* target, int attachment, const Rect& rect, void* data){
  if (target && !checkResources("readPixels"))
    return false;
  GLint previous = 0;
  if (resourcesSupported_){
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previous);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, target ? ((GLRenderTarget*)target)->getFbo() : 0);
    glReadBuffer(target ? GL_COLOR_ATTACHMENT0+attachment : GL_BACK);
  }
  glReadPixels(rect.x, rect.y, rect.width, rect.height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  if (resourcesSupported_)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, previous);
  return true;
}

// --- resources ---

std::unique_ptr<Buffer> GLDevice::createBuffer(const BufferDesc& desc, const void* data){
  TR_USE(CGE_Renderer_GL);
  if (!checkResources("createBuffer"))
    return nullptr;
  if (desc.size == 0){
    TR_ERROR("createBuffer: the size is 0");
    return nullptr;
  }
  if (desc.type == BufferType::Uniform && desc.size > (size_t)caps_.maxUniformBufferSize)
    TR_WARN("createBuffer: uniform buffer of %d bytes, but blocks can only have %d", (int)desc.size, caps_.maxUniformBufferSize);
  GLuint id = 0;
  glGenBuffers(1, &id);
  //the copy target, so neither the VAO's index buffer nor GL_ARRAY_BUFFER changes
  glBindBuffer(GL_COPY_WRITE_BUFFER, id);
  GLenum usage = desc.usage == Usage::Stream ? GL_STREAM_DRAW : desc.usage == Usage::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
  glBufferData(GL_COPY_WRITE_BUFFER, desc.size, data, usage);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
  return std::make_unique<GLBuffer>(desc, id);
}

void GLDevice::updateBuffer(Buffer* buffer, size_t offset, size_t size, const void* data){
  TR_USE(CGE_Renderer_GL);
  if (!buffer || !checkResources("updateBuffer"))
    return;
  const BufferDesc& desc = buffer->getDesc();
  if (desc.usage == Usage::Immutable){
    TR_ERROR("updateBuffer: the buffer is immutable");
    return;
  }
  if (offset+size > desc.size){
    TR_ERROR("updateBuffer: %d bytes at %d don't fit into %d", (int)size, (int)offset, (int)desc.size);
    return;
  }
  glBindBuffer(GL_COPY_WRITE_BUFFER, ((GLBuffer*)buffer)->getId());
  if (offset == 0 && size == desc.size){
    //new storage for the whole buffer, so the driver needn't wait for draws that still read the old data
    GLenum usage = desc.usage == Usage::Stream ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW;
    glBufferData(GL_COPY_WRITE_BUFFER, size, data, usage);
  }
  else
    glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

std::unique_ptr<GLTexture> GLDevice::makeTexture(const TextureDesc& desc, const void* data){
  TR_USE(CGE_Renderer_GL);
  GLFormat format;
  if (!toGLFormat(desc.format, format)){
    TR_ERROR("createTexture: no format");
    return nullptr;
  }
  if (desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize){
    TR_ERROR("createTexture: %dx%d is not a valid size", desc.width, desc.height);
    return nullptr;
  }
  int layers = 1;
  if (desc.type == TextureType::Texture2DArray){
    layers = desc.layers;
    if (layers < 1 || layers > caps_.maxTextureLayers){
      TR_ERROR("createTexture: %d is not a valid number of layers", layers);
      return nullptr;
    }
  }
  else if (desc.type == TextureType::Cube){
    if (desc.layers != 6 || desc.width != desc.height){
      TR_ERROR("createTexture: cube maps need 6 square layers");
      return nullptr;
    }
    layers = 6;
  }
  int levels = fullMipChain(desc.width, desc.height);
  if (desc.mipLevels > 0)
    levels = (std::min)(desc.mipLevels, levels);
  GLenum target = toGLTarget(desc.type);

  GLuint id = 0;
  glGenTextures(1, &id);
  auto texture = std::make_unique<GLTexture>(desc, id, target, levels);
  bindUploadTexture(texture.get());
  const uint8_t* pixels = (const uint8_t*)data;
  size_t layerSize = (size_t)desc.width*desc.height*formatSize(desc.format);
  for (int level = 0; level < levels; level++){
    int width = (std::max)(desc.width >> level, 1);
    int height = (std::max)(desc.height >> level, 1);
    //only level 0 gets data; the others are allocated
    const uint8_t* levelData = level == 0 ? pixels : nullptr;
    if (target == GL_TEXTURE_2D_ARRAY)
      glTexImage3D(target, level, format.internalFormat, width, height, layers, 0, format.format, format.type, levelData);
    else if (target == GL_TEXTURE_CUBE_MAP){
      for (int face = 0; face < 6; face++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, level, format.internalFormat, width, height, 0, format.format, format.type, levelData ? levelData+face*layerSize : nullptr);
    }
    else
      glTexImage2D(target, level, format.internalFormat, width, height, 0, format.format, format.type, levelData);
  }
  //complete with the levels it has
  glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels-1);
  endUpload();
  return texture;
}

std::unique_ptr<Texture> GLDevice::createTexture(const TextureDesc& desc, const void* data){
  if (!checkResources("createTexture"))
    return nullptr;
  return makeTexture(desc, data);
}

void GLDevice::updateTexture(Texture* texture, const TextureRegion& region, const void* data){
  TR_USE(CGE_Renderer_GL);
  if (!texture || !checkResources("updateTexture"))
    return;
  GLTexture* glTexture = (GLTexture*)texture;
  const TextureDesc& desc = texture->getDesc();
  int width = (std::max)(desc.width >> region.mipLevel, 1);
  int height = (std::max)(desc.height >> region.mipLevel, 1);
  int layers = desc.type == TextureType::Texture2D ? 1 : desc.layers;
  if (region.mipLevel < 0 || region.mipLevel >= glTexture->getLevels() || region.layer < 0 || region.layer >= layers
    || region.x < 0 || region.y < 0 || region.width <= 0 || region.height <= 0 || region.x+region.width > width || region.y+region.height > height){
    TR_ERROR("updateTexture: the region is outside the texture");
    return;
  }
  GLFormat format;
  toGLFormat(desc.format, format);
  bindUploadTexture(glTexture);
  GLenum target = glTexture->getTarget();
  if (target == GL_TEXTURE_2D_ARRAY)
    glTexSubImage3D(target, region.mipLevel, region.x, region.y, region.layer, region.width, region.height, 1, format.format, format.type, data);
  else if (target == GL_TEXTURE_CUBE_MAP)
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+region.layer, region.mipLevel, region.x, region.y, region.width, region.height, format.format, format.type, data);
  else
    glTexSubImage2D(target, region.mipLevel, region.x, region.y, region.width, region.height, format.format, format.type, data);
  endUpload();
}

void GLDevice::generateMipmaps(Texture* texture){
  if (!texture || !checkResources("generateMipmaps"))
    return;
  GLTexture* glTexture = (GLTexture*)texture;
  if (glTexture->getLevels() <= 1)
    return;
  bindUploadTexture(glTexture);
  glGenerateMipmap(glTexture->getTarget());
  endUpload();
}

std::unique_ptr<Sampler> GLDevice::createSampler(const SamplerDesc& desc){
  if (!checkResources("createSampler"))
    return nullptr;
  GLuint id = 0;
  glGenSamplers(1, &id);
  glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, toGLMinFilter(desc.minFilter, desc.mipFilter));
  glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, desc.magFilter == Filter::Linear ? GL_LINEAR : GL_NEAREST);
  glSamplerParameteri(id, GL_TEXTURE_WRAP_S, toGL(desc.addressU));
  glSamplerParameteri(id, GL_TEXTURE_WRAP_T, toGL(desc.addressV));
  glSamplerParameteri(id, GL_TEXTURE_WRAP_R, toGL(desc.addressW));
#if !defined(CGE_GLES)
  if (caps_.maxAnisotropy > 1 && desc.maxAnisotropy > 1)
    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, (std::min)(desc.maxAnisotropy, caps_.maxAnisotropy));
#endif
  return std::make_unique<GLSampler>(desc, id);
}

std::unique_ptr<RenderTarget> GLDevice::createRenderTarget(const RenderTargetDesc& desc){
  TR_USE(CGE_Renderer_GL);
  if (!checkResources("createRenderTarget"))
    return nullptr;
  int maxColors = (std::min)(MAX_COLOR_ATTACHMENTS, caps_.maxColorAttachments);
  if ((int)desc.colorFormats.size() > maxColors){
    TR_ERROR("createRenderTarget: %d colour attachments, at most %d are possible", (int)desc.colorFormats.size(), maxColors);
    return nullptr;
  }
  if (desc.depthFormat != Format::Undefined && !isDepthFormat(desc.depthFormat)){
    TR_ERROR("createRenderTarget: the depth attachment needs a depth format");
    return nullptr;
  }
  TextureDesc texDesc;
  texDesc.width = desc.width;
  texDesc.height = desc.height;
  texDesc.sampled = desc.sampled;
  texDesc.renderTarget = true;
  std::vector<std::unique_ptr<GLTexture> > colors;
  for (Format format : desc.colorFormats){
    if (isDepthFormat(format)){
      TR_ERROR("createRenderTarget: a colour attachment has a depth format");
      return nullptr;
    }
    texDesc.format = format;
    auto texture = makeTexture(texDesc, nullptr);
    if (!texture)
      return nullptr;
    colors.push_back(std::move(texture));
  }
  std::unique_ptr<GLTexture> depth;
  if (desc.depthFormat != Format::Undefined){
    texDesc.format = desc.depthFormat;
    depth = makeTexture(texDesc, nullptr);
    if (!depth)
      return nullptr;
  }

  //the current framebuffer is the pass's, or the old renderers' outside passes
  GLint previous = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous);
  GLuint fbo = 0;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  GLenum drawBuffers[MAX_COLOR_ATTACHMENTS];
  for (size_t i = 0; i < colors.size(); i++){
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+(GLenum)i, GL_TEXTURE_2D, colors[i]->getId(), 0);
    drawBuffers[i] = GL_COLOR_ATTACHMENT0+(GLenum)i;
  }
  if (depth){
    GLenum attachment = desc.depthFormat == Format::Depth24Stencil8 ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, depth->getId(), 0);
  }
  if (colors.empty()){
    GLenum none = GL_NONE;
    glDrawBuffers(1, &none);
    glReadBuffer(GL_NONE);
  }
  else
    glDrawBuffers((GLsizei)colors.size(), drawBuffers);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, previous);
  auto target = std::make_unique<GLRenderTarget>(desc, fbo, std::move(colors), std::move(depth));
  if (status != GL_FRAMEBUFFER_COMPLETE){
    TR_ERROR("createRenderTarget: the framebuffer is incomplete (0x%x)", status);
    return nullptr;
  }
  return target;
}

std::unique_ptr<Shader> GLDevice::createShader(const ShaderDesc& desc){
  TR_USE(CGE_Renderer_GL);
  if (!checkResources("createShader"))
    return nullptr;
  GLuint vertex = compileStage(GL_VERTEX_SHADER, desc.vertexSource, desc.name);
  if (!vertex)
    return nullptr;
  GLuint fragment = compileStage(GL_FRAGMENT_SHADER, desc.fragmentSource, desc.name);
  if (!fragment){
    glDeleteShader(vertex);
    return nullptr;
  }
  GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDetachShader(program, vertex);
  glDetachShader(program, fragment);
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  GLint ok = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok){
    TR_ERROR("Shader %s doesn't link:\n%s", desc.name.c_str(), infoLog(program, true).c_str());
    glDeleteProgram(program);
    return nullptr;
  }
  auto shader = std::make_unique<GLShader>(desc.name, program);

  //the bindings GLSL 3.30 / ES 3.00 can't declare in the source
  for (const SlotBinding& block : desc.uniformBlocks){
    if (block.slot < 0 || block.slot >= MAX_UNIFORM_BUFFERS){
      TR_ERROR("Shader %s: uniform block %s has the invalid slot %d", desc.name.c_str(), block.name.c_str(), block.slot);
      return nullptr;
    }
    GLuint index = glGetUniformBlockIndex(program, block.name.c_str());
    //the compiler removes unused blocks
    if (index == GL_INVALID_INDEX)
      TR_DEBUG("Shader %s doesn't use uniform block %s", desc.name.c_str(), block.name.c_str());
    else
      glUniformBlockBinding(program, index, block.slot);
  }
  if (!desc.textures.empty()){
    //sampler uniforms are set on the current program; the previous one (of a
    //pass, or of the old renderers) comes back afterwards
    GLint previous = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previous);
    glUseProgram(program);
    bool ok = true;
    for (const SlotBinding& texture : desc.textures){
      if (texture.slot < 0 || texture.slot >= MAX_TEXTURE_SLOTS){
        TR_ERROR("Shader %s: sampler %s has the invalid slot %d", desc.name.c_str(), texture.name.c_str(), texture.slot);
        ok = false;
        break;
      }
      GLint location = glGetUniformLocation(program, texture.name.c_str());
      if (location < 0)
        TR_DEBUG("Shader %s doesn't use sampler %s", desc.name.c_str(), texture.name.c_str());
      else
        glUniform1i(location, texture.slot);
    }
    glUseProgram(previous);
    if (!ok)
      return nullptr;
  }
  return shader;
}

std::unique_ptr<Pipeline> GLDevice::createPipeline(const PipelineDesc& desc){
  TR_USE(CGE_Renderer_GL);
  if (!checkResources("createPipeline"))
    return nullptr;
  if (!desc.shader){
    TR_ERROR("createPipeline %s: no shader", desc.name.c_str());
    return nullptr;
  }
  if (desc.raster.fill == FillMode::Wireframe && !caps_.wireframe){
    TR_ERROR("createPipeline %s: wireframe isn't supported", desc.name.c_str());
    return nullptr;
  }
  const VertexLayout& layout = desc.vertexLayout;
  for (const VertexAttribute& attr : layout.attributes){
    if (attr.location < 0 || attr.location >= (std::min)(MAX_VERTEX_ATTRIBUTES, caps_.maxVertexAttributes) || attr.bufferSlot < 0 || attr.bufferSlot >= MAX_VERTEX_BUFFERS || attr.offset < 0){
      TR_ERROR("createPipeline %s: attribute at location %d is invalid", desc.name.c_str(), attr.location);
      return nullptr;
    }
  }
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  auto pipeline = std::make_unique<GLPipeline>(desc, vao);
  //the attributes and their divisors are part of the VAO
  GLint previous = 0;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous);
  glBindVertexArray(vao);
  for (const VertexAttribute& attr : layout.attributes){
    glEnableVertexAttribArray(attr.location);
    bool perInstance = attr.bufferSlot < (int)layout.buffers.size() && layout.buffers[attr.bufferSlot].perInstance;
    glVertexAttribDivisor(attr.location, perInstance ? 1 : 0);
  }
  glBindVertexArray(previous);
  return pipeline;
}
