#ifndef CGE_LIGHT_H
#define CGE_LIGHT_H

#include "../math/vector.h"

namespace CGE{

class Light{
public:
  enum Type{
    Directional,
    Point,
    Spot
  };
  Light(Type t, const Vec3f val);
  ~Light();
  Type getType() const {return mType;}
  const Vec3f& getDirection() const {return mDirection;}
protected:
  Type mType;
  Vec3f mPosition;
  Vec3f mDirection;
};

}

#endif