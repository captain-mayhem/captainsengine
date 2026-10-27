#ifndef CGE_GL_RESOURCES_H
#define CGE_GL_RESOURCES_H

#include <memory>
#include <vector>

#include "../renderdevice.h"
#include "GLApi.h"

namespace CGE{
namespace Gfx{

//! a number that is unique for every GL object the backend creates
/*! The device's state cache compares these instead of GL names, because GL
    reuses the names of deleted objects. */
uint64_t nextGLSerial();
//! the serial of "not known", e.g. after other code changed the GL state
const uint64_t GL_UNKNOWN = ~(uint64_t)0;

//! the GL formats of a Format
struct GLFormat{
  GLenum internalFormat = 0;
  GLenum format = 0;
  GLenum type = 0;
};
//! false for Format::Undefined
bool toGLFormat(Format fmt, GLFormat& out);

// The resources delete their GL objects when they are deleted, so the
// device's context has to be current (it is, until the device shuts down).

class GLBuffer : public Buffer{
public:
  GLBuffer(const BufferDesc& desc, GLuint id) : Buffer(desc), id_(id), serial_(nextGLSerial()) {}
  ~GLBuffer();
  GLuint getId() const {return id_;}
  uint64_t getSerial() const {return serial_;}
private:
  GLuint id_;
  uint64_t serial_;
};

class GLTexture : public Texture{
public:
  //! levels is the resolved number of mip levels
  GLTexture(const TextureDesc& desc, GLuint id, GLenum target, int levels);
  ~GLTexture();
  GLuint getId() const {return id_;}
  GLenum getTarget() const {return target_;}
  int getLevels() const {return levels_;}
  uint64_t getSerial() const {return serial_;}
private:
  GLuint id_;
  GLenum target_;
  int levels_;
  uint64_t serial_;
};

class GLSampler : public Sampler{
public:
  GLSampler(const SamplerDesc& desc, GLuint id) : Sampler(desc), id_(id), serial_(nextGLSerial()) {}
  ~GLSampler();
  GLuint getId() const {return id_;}
  uint64_t getSerial() const {return serial_;}
private:
  GLuint id_;
  uint64_t serial_;
};

class GLShader : public Shader{
public:
  GLShader(const std::string& name, GLuint program) : Shader(name), program_(program), serial_(nextGLSerial()) {}
  ~GLShader();
  GLuint getProgram() const {return program_;}
  uint64_t getSerial() const {return serial_;}
  //! a uniform block the program uses: its slot and its size in bytes
  struct Block{
    int slot;
    int size;
  };
  std::vector<Block> blocks;
private:
  GLuint program_;
  uint64_t serial_;
};

//! a pipeline with its own vertex array object
/*! OpenGL 3.3 has no separate vertex buffer bindings, so the attribute
    pointers are set when drawing, and only where the bound buffer or offset
    differs from what the VAO already has. */
class GLPipeline : public Pipeline{
public:
  GLPipeline(const PipelineDesc& desc, GLuint vao);
  ~GLPipeline();
  GLuint getVao() const {return vao_;}
  GLenum getMode() const {return mode_;}
  uint64_t getSerial() const {return serial_;}
  GLShader* getShader() const {return (GLShader*)desc_.shader;}

  //! what the VAO's attribute pointer i points at
  struct AttributeSource{
    uint64_t buffer = GL_UNKNOWN;
    size_t offset = 0;
  };
  std::vector<AttributeSource> attributeSources;
  //! the serial of the VAO's index buffer
  uint64_t indexBuffer = GL_UNKNOWN;
private:
  GLuint vao_;
  GLenum mode_;
  uint64_t serial_;
};

class GLRenderTarget : public RenderTarget{
public:
  GLRenderTarget(const RenderTargetDesc& desc, GLuint fbo, std::vector<std::unique_ptr<GLTexture> > colors, std::unique_ptr<GLTexture> depth);
  ~GLRenderTarget();
  Texture* getColorTexture(int idx) const override;
  Texture* getDepthTexture() const override {return depth_.get();}
  GLuint getFbo() const {return fbo_;}
  bool hasStencil() const {return desc_.depthFormat == Format::Depth24Stencil8;}
private:
  GLuint fbo_;
  std::vector<std::unique_ptr<GLTexture> > colors_;
  std::unique_ptr<GLTexture> depth_;
};

}
}

#endif
