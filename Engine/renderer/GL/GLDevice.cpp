#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(ANDROID)
#include <GLES2/gl2.h>
#define glClearDepth glClearDepthf
#else
#include <GL/gl.h>
#endif

#include "GLDevice.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL(CGE_Renderer_GL);

RenderDevice* CGE::Gfx::createGLRenderDevice(){
  return new GLDevice();
}

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
  TR_INFO("OpenGL %s on %s", (const char*)glGetString(GL_VERSION), (const char*)glGetString(GL_RENDERER));
  context_->setSwapInterval(desc.vsync ? 1 : 0);
  hasStencil_ = desc.stencilBits > 0;

  GLint value = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
  caps_.maxTextureSize = value;
  //the rest needs the GL 3.3 / ES 3.0 context (step 2)
  caps_.maxTextureLayers = 1;
  caps_.maxColorAttachments = 1;
  caps_.maxVertexAttributes = 0;
  caps_.maxUniformBufferSize = 0;
  caps_.uniformBufferOffsetAlignment = 256;
  caps_.maxAnisotropy = 1;
#if defined(ANDROID)
  caps_.wireframe = false;
#else
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
