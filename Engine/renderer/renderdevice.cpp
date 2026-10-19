#include "renderdevice.h"

#include <cmath>
#include <map>

#include "../math/matrix.h"
#include "../io/Tracing.h"
#include "../system/soloader.h"
#include "renderbackend.h"
#include "nulldevice.h"
#ifdef CGE_GL_BUILTIN
#include "GL/GLDevice.h"
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(UNIX)
#include <dlfcn.h>
#endif

using namespace CGE;
using namespace CGE::Gfx;

TR_CHANNEL(CGE_RenderDevice);

Resource::~Resource(){
}

namespace{

//! the folder of the engine library, where the backend plugins are
std::string engineLibraryDir(){
#ifdef WIN32
  HMODULE module = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    (LPCSTR)&engineLibraryDir, &module))
    return "";
  char path[MAX_PATH];
  DWORD len = GetModuleFileNameA(module, path, MAX_PATH);
  if (len == 0 || len == MAX_PATH)
    return "";
  std::string dir(path, len);
  size_t pos = dir.find_last_of("\\/");
  return pos == std::string::npos ? "" : dir.substr(0, pos);
#elif defined(UNIX)
  Dl_info info;
  if (!dladdr((void*)&engineLibraryDir, &info) || !info.dli_fname)
    return "";
  std::string dir = info.dli_fname;
  size_t pos = dir.find_last_of('/');
  return pos == std::string::npos ? "" : dir.substr(0, pos);
#else
  return "";
#endif
}

//! loads a backend plugin; the libraries stay loaded until the process ends
RenderDevice* createFromPlugin(const std::string& library){
  TR_USE(CGE_RenderDevice);
  static std::map<std::string, std::unique_ptr<SOLoader> > plugins;
  std::unique_ptr<SOLoader>& plugin = plugins[library];
  if (!plugin){
    auto loader = std::make_unique<SOLoader>();
    std::string dir = engineLibraryDir();
    if (!loader->open(dir, library)){
      TR_ERROR("Render backend %s can't be loaded from %s: %s", library.c_str(), dir.c_str(), SOLoader::getLastError().c_str());
      return nullptr;
    }
    CgeRenderBackendVersionFunc version = (CgeRenderBackendVersionFunc)loader->getFunction(CGE_RENDER_BACKEND_VERSION_FUNC);
    if (!version || !loader->getFunction(CGE_RENDER_BACKEND_CREATE_FUNC)){
      TR_ERROR("%s is not a render backend", library.c_str());
      return nullptr;
    }
    if (version() != CGE_RENDER_BACKEND_VERSION){
      TR_ERROR("Render backend %s has version %d, the engine needs %d", library.c_str(), version(), CGE_RENDER_BACKEND_VERSION);
      return nullptr;
    }
    plugin = std::move(loader);
  }
  CgeCreateRenderDeviceFunc create = (CgeCreateRenderDeviceFunc)plugin->getFunction(CGE_RENDER_BACKEND_CREATE_FUNC);
  return create();
}

}

std::unique_ptr<RenderDevice> RenderDevice::create(const std::string& backend){
  TR_USE(CGE_RenderDevice);
  RenderDevice* device = nullptr;
  if (backend == "Null")
    device = new NullDevice();
  else if (backend == "OpenGL"){
#ifdef CGE_GL_BUILTIN
    device = createGLRenderDevice();
#else
    device = createFromPlugin("CGE_GL");
#endif
  }
  else
    TR_ERROR("Unknown render backend %s", backend.c_str());
  if (device)
    TR_INFO("Render backend %s", device->getName());
  return std::unique_ptr<RenderDevice>(device);
}

RenderDevice::RenderDevice(){
  width_ = 0;
  height_ = 0;
  state_ = State::Uninitialized;
  pipeline_ = nullptr;
  indexBufferSet_ = false;
}

RenderDevice::~RenderDevice(){
}

bool RenderDevice::checkState(State expected, const char* call) const{
  TR_USE(CGE_RenderDevice);
  if (state_ == expected)
    return true;
  static const char* names[] = {"uninitialised", "outside a frame", "in a frame", "in a pass"};
  TR_ERROR("%s called %s, but must be called %s", call, names[(int)state_], names[(int)expected]);
  return false;
}

bool RenderDevice::init(const NativeWindow& window, const DeviceDesc& desc){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::Uninitialized, "init"))
    return false;
  if (!doInit(window, desc)){
    TR_ERROR("Render device %s could not be initialised", getName());
    return false;
  }
  state_ = State::Idle;
  return true;
}

void RenderDevice::shutdown(){
  if (state_ == State::Uninitialized)
    return;
  if (state_ == State::InPass)
    endPass();
  if (state_ == State::InFrame)
    endFrame();
  doShutdown();
  state_ = State::Uninitialized;
}

void RenderDevice::resize(int width, int height){
  if (state_ == State::Uninitialized)
    return;
  width_ = width;
  height_ = height;
  doResize(width, height);
}

Matrix RenderDevice::adjustProjection(const Matrix& glProjection) const{
  if (!clipSpace_.depthZeroToOne && !clipSpace_.yDown)
    return glProjection;
  //applied after the projection: z' = (z+w)/2, y' = -y
  Matrix conv(Matrix::Identity);
  if (clipSpace_.depthZeroToOne){
    conv.at(10) = 0.5f;
    conv.at(14) = 0.5f;
  }
  if (clipSpace_.yDown)
    conv.at(5) = -1;
  return conv*glProjection;
}

Matrix RenderDevice::ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane) const{
  return adjustProjection(Matrix(Matrix::Ortho, left, right, bottom, top, nearPlane, farPlane));
}

Matrix RenderDevice::perspective(float fovy, float aspect, float nearPlane, float farPlane) const{
  float top = nearPlane*tanf(fovy*3.14159265f/360.0f);
  float right = top*aspect;
  return adjustProjection(Matrix(Matrix::Perspective, -right, right, -top, top, nearPlane, farPlane));
}

bool RenderDevice::beginFrame(){
  if (!checkState(State::Idle, "beginFrame"))
    return false;
  if (!doBeginFrame())
    return false;
  state_ = State::InFrame;
  return true;
}

void RenderDevice::endFrame(){
  if (state_ == State::InPass){
    TR_USE(CGE_RenderDevice);
    TR_ERROR("endFrame called in a pass");
    endPass();
  }
  if (!checkState(State::InFrame, "endFrame"))
    return;
  doEndFrame();
  state_ = State::Idle;
}

bool RenderDevice::beginPass(const PassDesc& desc){
  if (!checkState(State::InFrame, "beginPass"))
    return false;
  int width = width_;
  int height = height_;
  if (desc.target){
    width = desc.target->getDesc().width;
    height = desc.target->getDesc().height;
  }
  pipeline_ = nullptr;
  indexBufferSet_ = false;
  doBeginPass(desc, width, height);
  state_ = State::InPass;
  return true;
}

void RenderDevice::endPass(){
  if (!checkState(State::InPass, "endPass"))
    return;
  doEndPass();
  pipeline_ = nullptr;
  state_ = State::InFrame;
}

void RenderDevice::setPipeline(Pipeline* pipeline){
  if (!checkState(State::InPass, "setPipeline") || !pipeline)
    return;
  pipeline_ = pipeline;
  doSetPipeline(pipeline);
}

void RenderDevice::setVertexBuffer(int slot, Buffer* buffer, size_t offset){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::InPass, "setVertexBuffer"))
    return;
  if (slot < 0 || slot >= MAX_VERTEX_BUFFERS || (buffer && buffer->getDesc().type != BufferType::Vertex)){
    TR_ERROR("setVertexBuffer: slot %d or the buffer is invalid", slot);
    return;
  }
  doSetVertexBuffer(slot, buffer, offset);
}

void RenderDevice::setIndexBuffer(Buffer* buffer, IndexType type, size_t offset){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::InPass, "setIndexBuffer"))
    return;
  if (buffer && buffer->getDesc().type != BufferType::Index){
    TR_ERROR("setIndexBuffer: not an index buffer");
    return;
  }
  indexBufferSet_ = buffer != nullptr;
  doSetIndexBuffer(buffer, type, offset);
}

void RenderDevice::setUniformBuffer(int slot, Buffer* buffer, size_t offset, size_t size){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::InPass, "setUniformBuffer"))
    return;
  if (slot < 0 || slot >= MAX_UNIFORM_BUFFERS || (buffer && buffer->getDesc().type != BufferType::Uniform)){
    TR_ERROR("setUniformBuffer: slot %d or the buffer is invalid", slot);
    return;
  }
  if (buffer){
    if (offset % caps_.uniformBufferOffsetAlignment != 0 || offset+size > buffer->getDesc().size){
      TR_ERROR("setUniformBuffer: offset %d / size %d don't fit the buffer", (int)offset, (int)size);
      return;
    }
    if (size == 0)
      size = buffer->getDesc().size-offset;
  }
  doSetUniformBuffer(slot, buffer, offset, size);
}

void RenderDevice::setTexture(int slot, Texture* texture, Sampler* sampler){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::InPass, "setTexture"))
    return;
  if (slot < 0 || slot >= MAX_TEXTURE_SLOTS || (texture && !sampler)){
    TR_ERROR("setTexture: slot %d is invalid or the sampler is missing", slot);
    return;
  }
  doSetTexture(slot, texture, sampler);
}

void RenderDevice::setViewport(const Viewport& viewport){
  if (!checkState(State::InPass, "setViewport"))
    return;
  doSetViewport(viewport);
}

void RenderDevice::setScissor(const Rect& rect){
  if (!checkState(State::InPass, "setScissor"))
    return;
  doSetScissor(rect);
}

void RenderDevice::draw(int vertexCount, int instanceCount, int firstVertex){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::InPass, "draw"))
    return;
  if (!pipeline_){
    TR_ERROR("draw: no pipeline set");
    return;
  }
  if (vertexCount <= 0 || instanceCount <= 0)
    return;
  doDraw(vertexCount, instanceCount, firstVertex);
}

void RenderDevice::drawIndexed(int indexCount, int instanceCount, int firstIndex){
  TR_USE(CGE_RenderDevice);
  if (!checkState(State::InPass, "drawIndexed"))
    return;
  if (!pipeline_ || !indexBufferSet_){
    TR_ERROR("drawIndexed: no pipeline or no index buffer set");
    return;
  }
  if (indexCount <= 0 || instanceCount <= 0)
    return;
  doDrawIndexed(indexCount, instanceCount, firstIndex);
}
