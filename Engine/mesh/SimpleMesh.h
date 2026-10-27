#include <cge_export.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "../renderer/gfxtypes.h"

namespace CGE{

namespace Gfx{
class Buffer;
class DrawContext;
class RenderDevice;
}

//! Lit geometry without materials: a vertex buffer and maybe 16-bit indices
/*! It is drawn with the context's material and lights (BuiltinShader::Lit). */
class CGE_API SimpleMesh{
public:
  struct Vertex{
    float position[3];
    float normal[3];
  };
  //! the layout of Vertex: position and normal
  static const Gfx::VertexLayout& getVertexLayout();
  //! makes the buffers on the device; without indices the vertices are drawn in order
  SimpleMesh(Gfx::RenderDevice& device, const std::vector<Vertex>& vertices,
    const std::vector<uint16_t>& indices, Gfx::PrimitiveType primitive);
  ~SimpleMesh();
  void render(Gfx::DrawContext& ctx);
protected:
  std::unique_ptr<Gfx::Buffer> mVB;
  std::unique_ptr<Gfx::Buffer> mIB;
  int mCount;
  Gfx::PrimitiveType mRenderType;
};

}
