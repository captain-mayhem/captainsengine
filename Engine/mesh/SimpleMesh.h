#include <cge_export.h>

#include "../renderer/vertexbuffer.h"

namespace CGE{

class VertexBuffer;
class IndexBuffer;
class Camera;
class Renderer;

class CGE_API SimpleMesh{
public:
  SimpleMesh(VertexBuffer* vb, IndexBuffer* ib, PrimitiveType rt);
  ~SimpleMesh();
  void render();
protected:
  VertexBuffer* mVB;
  IndexBuffer* mIB;
  PrimitiveType mRenderType;
};

}

