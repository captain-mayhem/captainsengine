#ifndef ARCBALL_H
#define ARCBALL_H

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

namespace CGE{

class Arcball{
public:
  Arcball();
  ~Arcball();
  void click(const Vector2D& point);
  void drag(const Vector2D& point, Quaternion* rot);
  void update(const bool change, const bool reset, const Vector2D coords);
  Matrix getTrafo() const {return transform_;}
  Matrix getIncTrafo() const {return stepTransform_;}
protected:
  void mapToSphere(const Vector2D& point, Vector3D& newVec) const;
  Vector3D startVec_;
  Vector3D endVec_;
  bool isDragging_;
  Matrix lastRot_;
  Matrix currRot_;
  Matrix transform_;
  Matrix stepTransform_;
};

}

#endif

