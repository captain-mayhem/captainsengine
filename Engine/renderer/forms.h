#ifndef FORMS_H
#define FORMS_H

namespace Graphics{

class VertexBuffer;
  
class Forms{
public:
  Forms();
  ~Forms();
  void activateQuad();
  void drawQuad();
private:
  void constructVBOs();
  VertexBuffer* quad_;
};
  
}

#endif

