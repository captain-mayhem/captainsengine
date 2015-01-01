#ifndef FORMS_H
#define FORMS_H

namespace CGE{

class VertexBuffer;
  
class Forms{
public:
  Forms();
  ~Forms();
  void activateQuad();
  //! draw a unit quad
  /*! A unit quad reaches from -0.5 to 0.5 on each edge
  */
  void drawQuad();
  //! draw a quad at position position with the spanning of size
  void drawQuad(const CGE::Vector2D& position, const CGE::Vector2D& size, float rotation=0) const;

  void activateCylinder();
  void drawCylinder();
  static VertexBuffer* createCylinder(float radius, float height, int cylinder_segments, IndexBuffer** indices);
  static VertexBuffer* createBox(float width, float height, float depth, IndexBuffer** indices);
private:
  void constructVBOs();
  VertexBuffer* quad_;
  VertexBuffer* cylinder_;
  IndexBuffer* mCylinderInds[3];
};
  
}

#endif

