#include "gameobject.h"

#include "../math/matrix.h"

using namespace System;

GameObject::GameObject(){
  trafo_ = Math::Matrix(Math::Matrix::Identity);
}

GameObject::~GameObject(){
}

