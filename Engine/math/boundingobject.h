
#ifndef BOUNDINGOBJECT_H
#define BOUNDINGOBJECT_H

#include <cge_export.h>

namespace CGE{

class Ray;
class Matrix;
  
class CGE_API BoundingObject{
public:
  BoundingObject() {}
  virtual ~BoundingObject() {}
  virtual float hit(const Ray& r) const=0;
  virtual void transform(const Matrix& mat)=0;
  virtual BoundingObject* copy()=0;
};

}

#endif

