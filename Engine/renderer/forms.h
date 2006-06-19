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
  void Forms::drawQuad(const Vector2D& position, const Vector2D& size) const;
private:
  void constructVBOs();
  VertexBuffer* quad_;
};
  
}

#endif

