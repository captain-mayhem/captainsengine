#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(ANDROID)
#include <GLES2/gl2.h>
#define glClearDepth glClearDepthf
#else
#include <glad/gl.h>
#endif

#include "GLDevice.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL(CGE_Renderer_GL);

RenderDevice* CGE::Gfx::createGLRenderDevice(){
  return new GLDevice();
}

#if !defined(ANDROID)
namespace{

GLADapiproc loadGLFunction(void* context, const char* name){
  return (GLADapiproc)((GLContext*)context)->getProcAddress(name);
}

}
#endif

GLDevice::GLDevice(){
  context_ = GLContext::create();
  hasStencil_ = false;
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
#if defined(ANDROID)
  TR_INFO("OpenGL %s on %s", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_RENDERER));
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
  if (!GLAD_GL_VERSION_3_3)
    TR_WARN("Not an OpenGL 3.3 context: only the old renderers work on it");
#endif
  context_->setSwapInterval(desc.vsync ? 1 : 0);
  hasStencil_ = desc.stencilBits > 0;

  GLint value = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
  caps_.maxTextureSize = value;
#if defined(ANDROID)
  //the OpenGL ES 2.0 context; ES 3.0 comes with the Android port (Phase 1c)
  caps_.maxTextureLayers = 1;
  caps_.maxColorAttachments = 1;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
  caps_.maxVertexAttributes = value;
  caps_.maxUniformBufferSize = 0;
  caps_.uniformBufferOffsetAlignment = 256;
  caps_.maxAnisotropy = 1;
  caps_.wireframe = false;
#else
  caps_.maxTextureLayers = 1;
  caps_.maxColorAttachments = 1;
  caps_.maxUniformBufferSize = 0;
  caps_.uniformBufferOffsetAlignment = 256;
  if (GLAD_GL_VERSION_3_1){
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &value);
    caps_.maxTextureLayers = value;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &value);
    caps_.maxColorAttachments = value;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &value);
    caps_.maxUniformBufferSize = value;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    caps_.uniformBufferOffsetAlignment = value;
  }
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
  caps_.maxVertexAttributes = value;
  caps_.maxAnisotropy = 1;
  if (GLAD_GL_ARB_texture_filter_anisotropic || GLAD_GL_EXT_texture_filter_anisotropic){
    GLfloat anisotropy = 1;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropy);
    caps_.maxAnisotropy = anisotropy;
  }
  caps_.wireframe = true;
#endif
  //OpenGL's conventions are the reference, so nothing differs
  clipSpace_ = ClipSpace();
  return true;
}

void GLDevice::doShutdown(){
  context_->shutdown();
}

void GLDevice::setVsync(bool vsync){
  if (isInitialized())
    context_->setSwapInterval(vsync ? 1 : 0);
}

void GLDevice::doEndFrame(){
  context_->swapBuffers();
}

void GLDevice::doBeginPass(const PassDesc& desc, int width, int height){
  TR_USE(CGE_Renderer_GL);
  if (desc.target)
    TR_ERROR("Render targets aren't implemented in the GL backend yet; drawing into the window");
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  glEnable(GL_SCISSOR_TEST);
  GLbitfield mask = 0;
  if (desc.colorLoad == LoadAction::Clear){
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(desc.clearColor.r, desc.clearColor.g, desc.clearColor.b, desc.clearColor.a);
    mask |= GL_COLOR_BUFFER_BIT;
  }
  if (desc.depthLoad == LoadAction::Clear){
    glDepthMask(GL_TRUE);
    glClearDepth(desc.clearDepth);
    mask |= GL_DEPTH_BUFFER_BIT;
    if (hasStencil_){
      glStencilMask(0xFF);
      glClearStencil(desc.clearStencil);
      mask |= GL_STENCIL_BUFFER_BIT;
    }
  }
  if (mask)
    glClear(mask);
}

void GLDevice::doSetViewport(const Viewport& viewport){
  glViewport((GLint)viewport.x, (GLint)viewport.y, (GLsizei)viewport.width, (GLsizei)viewport.height);
}

void GLDevice::doSetScissor(const Rect& rect){
  glScissor(rect.x, rect.y, rect.width, rect.height);
}

void GLDevice::notYet(const char* what){
  TR_USE(CGE_Renderer_GL);
  TR_ERROR("%s aren't implemented in the GL backend yet", what);
}

std::unique_ptr<Buffer> GLDevice::createBuffer(const BufferDesc& desc, const void* data){
  notYet("Buffers");
  return nullptr;
}

std::unique_ptr<Texture> GLDevice::createTexture(const TextureDesc& desc, const void* data){
  notYet("Textures");
  return nullptr;
}

std::unique_ptr<Sampler> GLDevice::createSampler(const SamplerDesc& desc){
  notYet("Samplers");
  return nullptr;
}

std::unique_ptr<RenderTarget> GLDevice::createRenderTarget(const RenderTargetDesc& desc){
  notYet("Render targets");
  return nullptr;
}

std::unique_ptr<Shader> GLDevice::createShader(const ShaderDesc& desc){
  notYet("Shaders");
  return nullptr;
}

std::unique_ptr<Pipeline> GLDevice::createPipeline(const PipelineDesc& desc){
  notYet("Pipelines");
  return nullptr;
}

void GLDevice::updateBuffer(Buffer* buffer, size_t offset, size_t size, const void* data){
  notYet("Buffers");
}

void GLDevice::updateTexture(Texture* texture, const TextureRegion& region, const void* data){
  notYet("Textures");
}

void GLDevice::generateMipmaps(Texture* texture){
  notYet("Textures");
}
