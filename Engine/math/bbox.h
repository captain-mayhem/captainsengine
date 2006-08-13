#ifndef BBOX_H
#define BBOX_H

#include "vector.h"

namespace Math{

class Ray;
  
class BBox{
public:
  bool hit(const Ray& r) const;
private:
  Vector3D min_;
  Vector3D max_;
};

}

#endif
