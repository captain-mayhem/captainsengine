#ifndef BSPHERE_H
#define BSPHERE_H

#include "boundingobject.h"
#include "vector.h"

namespace Math{

class BSphere : public BoundingObject {
public:
  BSphere();
  BSphere(const Vector3D& center, const float radius);
  virtual ~BSphere();
  virtual float hit(const Ray& r) const;
  virtual void transform(const Matrix& mat);
  virtual BoundingObject* copy();
  Vec3f getCenter() const {return center_;}
  float getRadius() const {return radius_;}
protected:
  Vec3f center_;
  float radius_;
};

}

#endif

