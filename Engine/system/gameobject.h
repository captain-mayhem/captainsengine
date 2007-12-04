#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "../math/matrix.h"

namespace System{

class GameObject{
public:
  //! Constructor
  GameObject();
  //! Copy constructor
  GameObject(GameObject const& g);
  //! Destructor
  virtual ~GameObject();
  //! set transformation
  inline void setTrafo(const Math::Matrix& mat) {trafo_ = mat;}
  //! get transformation
  inline Math::Matrix getTrafo() const {return trafo_;}
  //! get id
  inline unsigned int getID() const {return id_;}
  //! set id
  void setID(unsigned id) {id_ = id;}
  //! set id count
  static void setIDCount(unsigned start) {idCount_ = start;}
  //! get id count
  static unsigned getIDCount(){return idCount_;}
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

