#ifndef CGE_MATRIX_STACK_H
#define CGE_MATRIX_STACK_H

#include <cge_export.h>

#include <vector>

#include "matrix.h"

namespace CGE{

//! A CPU-side matrix stack
/*! Replaces the old renderers' matrix stacks (pushMatrix, translate, rotate,
    ...). Like those, the operations multiply from the right, so the last
    transformation applied is the first one the vertices see. The top goes
    into a uniform buffer (e.g. Gfx::DrawContext's model matrix). */
class CGE_API MatrixStack{
public:
  //! a stack holding the identity
  MatrixStack();
  //! the current matrix
  const Matrix& top() const {return stack_.back();}
  //! copies the current matrix onto the stack
  void push();
  //! goes back to the matrix of the matching push; the last matrix stays
  void pop();
  //! the number of matrices on the stack (1 without pushes)
  int depth() const {return (int)stack_.size();}
  //! replaces the current matrix
  void load(const Matrix& mat);
  void loadIdentity();
  //! current = current * mat
  void multiply(const Matrix& mat);
  void translate(float x, float y, float z);
  //! a rotation by angle degrees around the axis (x, y, z)
  void rotate(float angle, float x, float y, float z);
  void scale(float x, float y, float z);
private:
  std::vector<Matrix> stack_;
};

}

#endif
