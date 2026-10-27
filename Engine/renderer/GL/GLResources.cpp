#include "GLResources.h"

using namespace CGE::Gfx;

uint64_t CGE::Gfx::nextGLSerial(){
  //the GL backend is used from one thread, the one with the context
  static uint64_t serial = 0;
  return ++serial;
}

bool CGE::Gfx::toGLFormat(Format fmt, GLFormat& out){
  switch (fmt){
    case Format::R8: out = {GL_R8, GL_RED, GL_UNSIGNED_BYTE}; return true;
    case Format::RG8: out = {GL_RG8, GL_RG, GL_UNSIGNED_BYTE}; return true;
    case Format::RGB8: out = {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE}; return true;
    case Format::RGBA8: out = {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}; return true;
    case Format::SRGBA8: out = {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE}; return true;
    case Format::R16F: out = {GL_R16F, GL_RED, GL_HALF_FLOAT}; return true;
    case Format::RG16F: out = {GL_RG16F, GL_RG, GL_HALF_FLOAT}; return true;
    case Format::RGBA16F: out = {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT}; return true;
    case Format::R32F: out = {GL_R32F, GL_RED, GL_FLOAT}; return true;
    case Format::RG32F: out = {GL_RG32F, GL_RG, GL_FLOAT}; return true;
    case Format::RGBA32F: out = {GL_RGBA32F, GL_RGBA, GL_FLOAT}; return true;
    case Format::Depth16: out = {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT}; return true;
    case Format::Depth24: out = {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT}; return true;
    case Format::Depth32F: out = {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}; return true;
    case Format::Depth24Stencil8: out = {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8}; return true;
    default: return false;
  }
}

GLBuffer::~GLBuffer(){
  glDeleteBuffers(1, &id_);
}

GLTexture::GLTexture(const TextureDesc& desc, GLuint id, GLenum target, int levels) : Texture(desc){
  id_ = id;
  target_ = target;
  levels_ = levels;
  serial_ = nextGLSerial();
}

GLTexture::~GLTexture(){
  glDeleteTextures(1, &id_);
}

GLSampler::~GLSampler(){
  glDeleteSamplers(1, &id_);
}

GLShader::~GLShader(){
  glDeleteProgram(program_);
}

namespace{

GLenum toGLMode(PrimitiveType type){
  switch (type){
    case PrimitiveType::Points: return GL_POINTS;
    case PrimitiveType::Lines: return GL_LINES;
    case PrimitiveType::LineStrip: return GL_LINE_STRIP;
    case PrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
    default: return GL_TRIANGLES;
  }
}

}

GLPipeline::GLPipeline(const PipelineDesc& desc, GLuint vao) : Pipeline(desc){
  vao_ = vao;
  mode_ = toGLMode(desc.primitive);
  serial_ = nextGLSerial();
  attributeSources.resize(desc.vertexLayout.attributes.size());
}

GLPipeline::~GLPipeline(){
  glDeleteVertexArrays(1, &vao_);
}

GLRenderTarget::GLRenderTarget(const RenderTargetDesc& desc, GLuint fbo, std::vector<std::unique_ptr<GLTexture> > colors, std::unique_ptr<GLTexture> depth)
  : RenderTarget(desc), fbo_(fbo), colors_(std::move(colors)), depth_(std::move(depth)){
}

GLRenderTarget::~GLRenderTarget(){
  glDeleteFramebuffers(1, &fbo_);
}

Texture* GLRenderTarget::getColorTexture(int idx) const{
  return idx >= 0 && idx < (int)colors_.size() ? colors_[idx].get() : nullptr;
}
