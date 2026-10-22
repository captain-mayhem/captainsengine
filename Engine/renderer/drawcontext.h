#ifndef CGE_DRAW_CONTEXT_H
#define CGE_DRAW_CONTEXT_H

#include <cge_export.h>

#include <memory>
#include <vector>

#include "gfxtypes.h"
#include "material.h"
#include "shaderlibrary.h"

namespace CGE{

class Light;
class MatrixStack;

namespace Gfx{

class Buffer;
class Pipeline;
class RenderDevice;
class Sampler;
class Shader;
class Texture;
class UniformStream;

enum class BlendMode{
  //! no blending
  Opaque,
  //! src*alpha + dst*(1-alpha)
  Alpha,
  //! src*alpha + dst
  Additive,
  //! src + dst*(1-alpha), for colours already multiplied by their alpha
  PremultipliedAlpha,
  //! src*dst, e.g. darkening or masking what is drawn
  Multiply,
};

//! What the engine's drawing code draws with on a render device
/*! Keeps the state the old renderers kept (projection, view, a model matrix
    stack, colour, material, lights, texture, blending and depth testing)
    and turns it into what the device needs: the engine's uniform blocks
    (CGE_Frame, CGE_Draw, CGE_Material, CGE_Lights) in a UniformStream, and
    pipelines, which are created on first use and kept.
    Forms, Mesh, Model, Scene and the render techniques draw through it.
    It doesn't begin or end passes; prepare and draw need a pass. */
class CGE_API DrawContext{
public:
  explicit DrawContext(RenderDevice& device);
  //! deletes the pipelines, shaders and buffers it made; before the device goes
  ~DrawContext();
  DrawContext(const DrawContext&) = delete;
  DrawContext& operator=(const DrawContext&) = delete;

  RenderDevice& getDevice() const;
  //! per-draw uniform data, e.g. for further blocks of own shaders
  UniformStream& getUniformStream();

  // --- CGE_Frame ---

  //! a projection for the device's clip space (RenderDevice::ortho, perspective)
  void setProjection(const Matrix& projection);
  const Matrix& getProjection() const;
  void setView(const Matrix& view);
  const Matrix& getView() const;
  void setTime(float seconds);

  // --- CGE_Draw ---

  //! the model matrix stack; its top is the model matrix of the draws
  MatrixStack& model();
  void setColor(float r, float g, float b, float a = 1);
  void setColor(const Color& color);
  const Color& getColor() const;

  // --- CGE_Material and CGE_Lights, for lit draws ---

  //! the material's colours, opacity and shininess (not its textures)
  void setMaterial(const Material& material);
  void setMaterial(const MaterialUniforms& material);
  //! white, unlit by ambient light, not shiny
  void resetMaterial();
  //! the lights in world space; the first MAX_LIGHTS are used
  /*! They're copied, and taken into view space with the view at the next
      draw. */
  void setLights(const std::vector<Light*>& lights);
  void clearLights();
  int getNumLights() const;
  //! writes a light into entry index of uniforms, in the view space of view
  static void setLightUniforms(LightUniforms& uniforms, int index, const Light& light, const Matrix& view);
  //! the shader that lit draws use instead of BuiltinShader::Lit; nullptr for that
  /*! It has to take the same vertex inputs and blocks; render techniques
      use it, e.g. for the light prepass's G-buffer. */
  void setLitShader(Shader* shader);

  // --- texture and pipeline state ---

  //! the texture in SLOT_TEXTURE; nullptr for none (untextured lit draws)
  /*! Without a sampler, a linear one that repeats is used. */
  void setTexture(Texture* texture, Sampler* sampler = nullptr);
  Texture* getTexture() const;
  //! the sampler given with the texture; nullptr for the default one
  Sampler* getSampler() const;
  void setBlend(BlendMode mode);
  BlendMode getBlend() const;
  //! depth testing and writing; both off by default
  void setDepth(bool test, bool write, CompareFunc compare = CompareFunc::LessEqual);
  //! writes to the colour attachments; on by default (off: depth only)
  void setColorWrite(bool write);
  void setCull(CullMode cull);

  // --- drawing, inside a pass ---

  //! the built-in shader (Lit: the one setLitShader set), created on first use
  Shader* getShader(BuiltinShader shader);
  //! the pipeline of a shader and a vertex layout with the current blend, depth and cull state
  /*! Created on first use and kept until forgetShader or the context's end. */
  Pipeline* getPipeline(Shader* shader, const VertexLayout& layout, PrimitiveType primitive);
  //! deletes the pipelines made for shader; call it before deleting a shader used here
  void forgetShader(Shader* shader);
  //! sets the pipeline and binds the engine's uniform blocks and the texture
  /*! For own shaders: bind further resources, then draw on the device. */
  bool prepare(Shader* shader, const VertexLayout& layout, PrimitiveType primitive);
  bool prepare(BuiltinShader shader, const VertexLayout& layout, PrimitiveType primitive);
  //! draws vertex buffer slot 0 with a built-in shader
  void draw(BuiltinShader shader, Buffer* vertices, const VertexLayout& layout, PrimitiveType primitive,
    int vertexCount, int firstVertex = 0);
  void drawIndexed(BuiltinShader shader, Buffer* vertices, const VertexLayout& layout, Buffer* indices,
    IndexType indexType, PrimitiveType primitive, int indexCount, int firstIndex = 0);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
}

#endif
