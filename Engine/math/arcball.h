#ifndef ARCBALL_H
#define ARCBALL_H

#include "vector.h"
#include "quaternion.h"

namespace Math{

class Arcball{
public:
  Arcball();
  ~Arcball();
  void click(const Vector2D& point);
  void drag(const Vector2D& point, Quaternion* rot);
protected:
  void mapToSphere(const Vector2D& point, Vector3D& newVec) const;
  Vector3D startVec_;
  Vector3D endVec_;
};

}

#endif

