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
  inline Math::Matrix getTrafo() const {return trafo_;}
  //! get id
  inline unsigned int getID() const {return id_;}
protected:
  //! The id counter
  static unsigned int idCount_;
  //! The object id
  unsigned int id_;
  //! The transformation
  Math::Matrix trafo_;
};

}

#endif

