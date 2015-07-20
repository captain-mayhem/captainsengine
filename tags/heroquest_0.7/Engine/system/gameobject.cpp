#include "gameobject.h"

#include "../math/matrix.h"

using namespace System;

unsigned int GameObject::idCount_ = 0;

GameObject::GameObject(){
  trafo_ = Math::Matrix(Math::Matrix::Identity);
  id_ = ++idCount_;
}

GameObject::~GameObject(){
}

