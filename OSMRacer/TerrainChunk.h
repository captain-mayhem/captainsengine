#include <system/types.h>

namespace Graphics{
class Camera;
class VertexBuffer;
}

class TerrainChunk{
public:
  TerrainChunk();
  ~TerrainChunk();
  void generate(int32 numVertices);
  void render(const Graphics::Camera& cam);
protected:
  Graphics::VertexBuffer* mVB;
};