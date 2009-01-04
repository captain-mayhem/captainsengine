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
  virtual float hit(const Ray& r) const;
  virtual void transform(const Matrix& mat);
  virtual BoundingObject* copy();
  Vec3f getMin() const {return min_;}
  Vec3f getMax() const {return max_;}
  Vec3f getCenter() const {return (max_+min_)/2;}
  Vec3f getSpan() const {return (max_-min_)/2;}
  Vec3f getNearestCorner(const Vec3f& normal) const;
  Vec3f getFarestCorner(const Vec3f& normal) const;
private:
  Vec3f min_;
  Vec3f max_;
};

}

#endif
