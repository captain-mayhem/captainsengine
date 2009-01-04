//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-12-24                                                       |
// |------------------------------------------------------------------|
// | File: plane.h                                                    |
//  ==================================================================

#ifndef PLANE_H
#define PLANE_H

#include "vector.h"

namespace Math{

class Plane{
public:
  Plane();
  Plane(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3);
  ~Plane();
  void setPoints(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3);
  float distance(const Vec3f& p) const;
  inline const Vec3f& getNormal() const {return mNormal;}
protected:
  Vec3f mNormal;
  Vec3f mPoint;
  float mD;
};

}

#endif
