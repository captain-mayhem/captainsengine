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
  Light(const Vec3f& pos, const Vec3f& dir, float cutoff);
  ~Light();
  Type getType() const {return mType;}
  const Vec4f& getPosition() const { return mPosition; }
  const Vec3f& getDirection() const { return mSpotDirection; }
  float getCutoff() const { return mSpotCutoff; }
protected:
  Type mType;
  Vec4f mPosition;
  Vec3f mSpotDirection;
  float mSpotCutoff;
};

}

#endif

