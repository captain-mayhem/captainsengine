
namespace CGE{

class VertexBuffer;
class IndexBuffer;
class Camera;
class Renderer;
enum PrimitiveType;

class SimpleMesh{
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