#include <system/types.h>
#include <math/bbox.h>

namespace Graphics{
class Camera;
class VertexBuffer;
}

class TerrainChunk{
public:
  TerrainChunk();
  ~TerrainChunk();
  void generate(int32 numVertices, float widthScale=1.0f, float startX=0, float startZ=0);
  void render(const Graphics::Camera& cam);
  CGE::BBox getBox();
protected:
  CGE::BBox mBox;
  Graphics::VertexBuffer* mVB;
};