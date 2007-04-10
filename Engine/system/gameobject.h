#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "../math/matrix.h"

namespace System{

class GameObject{
public:
  GameObject();
  virtual ~GameObject();
  //! set transformation
  inline void setTrafo(const Math::Matrix& mat) {trafo_ = mat;}
  //! get transformation
protected:
  Math::Matrix trafo_;
};

}

#endif

