

#include "SimpleMesh.h"

#include "../renderer/drawcontext.h"
#include "../renderer/renderdevice.h"

using namespace CGE;

const Gfx::VertexLayout& SimpleMesh::getVertexLayout(){
  using namespace Gfx;
  static const VertexLayout layout = {
    {
      {ATTR_POSITION, VertexFormat::Float3, 0, 0},
      {ATTR_NORMAL, VertexFormat::Float3, 0, 12},
    },
    {{(int)sizeof(Vertex), false}},
  };
  return layout;
}

SimpleMesh::SimpleMesh(Gfx::RenderDevice& device, const std::vector<Vertex>& vertices,
  const std::vector<uint16_t>& indices, Gfx::PrimitiveType primitive){
  using namespace Gfx;
  mVB = device.createBuffer({BufferType::Vertex, Usage::Immutable, vertices.size()*sizeof(Vertex)}, vertices.data());
  if (!indices.empty())
    mIB = device.createBuffer({BufferType::Index, Usage::Immutable, indices.size()*sizeof(uint16_t)}, indices.data());
  mCount = (int)(indices.empty() ? vertices.size() : indices.size());
  mRenderType = primitive;
}

SimpleMesh::~SimpleMesh(){

}

void SimpleMesh::render(Gfx::DrawContext& ctx){
  if (!mVB)
    return;
  if (mIB)
    ctx.drawIndexed(Gfx::BuiltinShader::Lit, mVB.get(), getVertexLayout(), mIB.get(), Gfx::IndexType::UInt16, mRenderType, mCount);
  else
    ctx.draw(Gfx::BuiltinShader::Lit, mVB.get(), getVertexLayout(), mRenderType, mCount);
}
