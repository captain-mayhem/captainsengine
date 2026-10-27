#include <memory>
#include <vector>

#include <system/types.h>
#include <math/bbox.h>

namespace CGE{
class Camera;
namespace Gfx{
class Buffer;
class DrawContext;
}
}

class TerrainChunk{
public:
  TerrainChunk();
  ~TerrainChunk();
  void generate(int32 numVertices, float widthScale=1.0f, float startX=0, float startZ=0);
  void render(CGE::Gfx::DrawContext& ctx, const CGE::Camera& cam);
  CGE::BBox getBox();
protected:
  CGE::BBox mBox;
  std::unique_ptr<CGE::Gfx::Buffer> mVB;
  //! a triangle strip for each level of detail
  std::vector<std::unique_ptr<CGE::Gfx::Buffer>> mInds;
  std::vector<int> mIndexCounts;
};