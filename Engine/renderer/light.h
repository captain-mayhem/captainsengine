#ifndef CGE_LIGHT_H
#define CGE_LIGHT_H

#include "../math/vector.h"
#include "material.h"

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
  void setPosition(Vec4f const& pos) { mPosition = pos; }
  const Vec4f& getPosition() const { return mPosition; }
  void setDirection(Vec3f const& dir) { mSpotDirection = dir; }
  const Vec3f& getDirection() const { return mSpotDirection; }
  void setCutoff(float cutoff) { mSpotCutoff = cutoff; }
  float getCutoff() const { return mSpotCutoff; }
  void setAttenuation(float att) { mAttenuation = att; }
  float getAttenuation() const { return mAttenuation; }
  void setColor(Color const& col) { mColor = col; }
  Color const& getColor() const { return mColor; }
protected:
  Type mType;
  Vec4f mPosition;
  Vec3f mSpotDirection;
  float mSpotCutoff;
  float mAttenuation;
  Color mColor;
};

}

#endif

