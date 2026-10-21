#include "matrixstack.h"

#include "../io/Tracing.h"

using namespace CGE;

TR_CHANNEL(CGE_MatrixStack);

MatrixStack::MatrixStack(){
  stack_.push_back(Matrix(Matrix::Identity));
}

void MatrixStack::push(){
  stack_.push_back(stack_.back());
}

void MatrixStack::pop(){
  if (stack_.size() <= 1){
    TR_USE(CGE_MatrixStack);
    TR_ERROR("pop without a matching push");
    return;
  }
  stack_.pop_back();
}

void MatrixStack::load(const Matrix& mat){
  stack_.back() = mat;
}

void MatrixStack::loadIdentity(){
  stack_.back() = Matrix(Matrix::Identity);
}

void MatrixStack::multiply(const Matrix& mat){
  stack_.back() *= mat;
}

void MatrixStack::translate(float x, float y, float z){
  stack_.back() *= Matrix(Matrix::Translation, Vec3f(x, y, z));
}

void MatrixStack::rotate(float angle, float x, float y, float z){
  stack_.back() *= Matrix(Matrix::Rotation, Vec3f(x, y, z), angle/180.0f*3.14159265358979f);
}

void MatrixStack::scale(float x, float y, float z){
  stack_.back() *= Matrix(Matrix::Scale, Vec3f(x, y, z));
}
