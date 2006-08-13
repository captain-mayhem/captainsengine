#ifndef RAY_H
#define RAY_H

#include "vector.h"

namespace Math{
  
class Ray{
public:
  Ray();
  Ray(const Vector3D& orig, const Vector3D dir);
  inline Vector3D getOrigin() const {return origin_;}
  inline Vector3D getDirection() const {return direction_;}
private:
  //! the origin of the ray
  Vector3D origin_;
  //! the direction of the ray
  Vector3D direction_;
};

}

#endif

