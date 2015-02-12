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
  const Vec4f& getPosition() const { return mPosition; }
protected:
  Type mType;
  Vec4f mPosition;
};

}

#endif

