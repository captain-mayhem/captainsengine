//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: gameobject.hh                                              |
//  ==================================================================

#ifndef GAMEOBJECT_HH
#define GAMEOBJECT_HH

#include "math/vector.h"

using ::CGE::Vector2D;

//! The base class for renderable objects in the game
class GameObject {
public:
  //! Constructor
  GameObject();
  //! Destructor
  virtual ~GameObject();
  //! Sets the position of the object
  inline void setPosition(const Vector2D& pos, bool interpolate=false){position_ = pos;interpolate_=interpolate;}
  //! returns the position of the object
  inline const Vector2D& getPosition() const {return position_;}
  //! sets, if the object is active
  inline void setActive(bool flag = true){active_ = flag;}
  //! returns, if the object is active
  inline const bool getStatus() const {return active_;}
  //! sets the object id
  inline void setId(const short id){id_ = id;}
  //! returns the object id
  inline const short getId() const {return id_;}
  //! renders the object in 3D
  virtual void render() const = 0;
  //! renders the object in 2D
  virtual void render2D() const = 0;
  //! updates object status
  virtual void update() = 0;
protected:
  //! the position
  Vector2D position_;
  //! object id
  short id_;
  //! status of the object
  bool active_;
  //! should positions be interpolated
  bool interpolate_;
};
  

#endif
