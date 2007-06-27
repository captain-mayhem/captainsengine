
#ifndef BOUNDINGOBJECT_H
#define BOUNDINGOBJECT_H

namespace Math{

class Ray;
class Matrix;
  
class BoundingObject{
public:
  BoundingObject() {}
  virtual ~BoundingObject() {}
  virtual bool hit(const Ray& r) const=0;
  virtual void transform(const Matrix& mat)=0;
  virtual BoundingObject* copy()=0;
};

}

#endif

