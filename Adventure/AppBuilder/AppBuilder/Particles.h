#ifndef ADV_PARTICLES_H
#define ADV_PARTICLES_H

#include <string>

#include "Vector.h"

class Object2D;
class AdvDocument;

class ParticleEngine{
public:
  ParticleEngine(AdvDocument* data);
  ~ParticleEngine();
  void update(unsigned time);
  void setParticleObject(const std::string& name);
  void setDirection(Vec2f dir) {mDir = dir;}
  void setMaxParticles(int maxParticles) {mMaxParticles = maxParticles;}
  void setRotation(float angle) {mRotAngle = angle;}
  void setSpeedVariation(float variation) {mSpeedVariation = variation;}
  void activate(bool doit, bool immediately);
protected:
  AdvDocument* mData;
  Object2D* mParticleObject;
  Vec2f mDir;
  int mMaxParticles;
  float mRotAngle;
  float mSpeedVariation;
  bool mEnabled;
};

#endif
