#ifndef FORMS_H
#define FORMS_H

namespace Graphics{

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
  void drawQuad(const CGE::Vector2D& position, const CGE::Vector2D& size) const;

  void activateCylinder();
  void drawCylinder();
  static VertexBuffer* createCylinder(float radius, float height, int cylinder_segments);
  static VertexBuffer* createBox(float width, float height, float depth);
private:
  void constructVBOs();
  VertexBuffer* quad_;
  VertexBuffer* cylinder_;
};
  
}

#endif

