#ifndef RAY_H
#define RAY_H

#include "vector.h"

namespace Math{

class Matrix;

class Ray{
public:
  Ray();
  Ray(const Vector3D& orig, const Vector3D dir);
  inline Vector3D getOrigin() const {return origin_;}
  inline Vector3D getDirection() const {return direction_;}
  //! generate a picking ray
  /* x and y are the screen coordinates, width and height the viewport dimensions
   * and nww/nwh are nearplane weighted width and nearplane weighted height
   */
  void buildPickingRay(float x, float y, float width, float height, float nww, float nwh, float nwd);
  //! transform a ray
  void transform(const Matrix& mat);
private:
  //! the origin of the ray
  Vector3D origin_;
  //! the direction of the ray
  Vector3D direction_;
};

}

#endif

