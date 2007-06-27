#ifndef BBOX_H
#define BBOX_H

#include "boundingobject.h"
#include "vector.h"

namespace Math{
  
class BBox : public BoundingObject {
public:
  BBox();
  BBox(const Vector3D& min, const Vector3D& max);
  virtual ~BBox();
  virtual bool hit(const Ray& r) const;
  virtual void transform(const Matrix& mat);
  virtual BoundingObject* copy();
  Vector3D getMin() const {return min_;}
  Vector3D getMax() const {return max_;}
private:
  Vector3D min_;
  Vector3D max_;
};

}

#endif
