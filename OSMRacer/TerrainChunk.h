#include <system/types.h>
#include <math/bbox.h>

namespace CGE{
class Camera;
class VertexBuffer;
class IndexBuffer;
}

class TerrainChunk{
public:
  TerrainChunk();
  ~TerrainChunk();
  void generate(int32 numVertices, float widthScale=1.0f, float startX=0, float startZ=0);
  void render(const CGE::Camera& cam);
  CGE::BBox getBox();
protected:
  CGE::BBox mBox;
  CGE::VertexBuffer* mVB;
  CGE::IndexBuffer** mInds;
};