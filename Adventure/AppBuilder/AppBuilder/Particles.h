#ifndef ADV_PARTICLES_H
#define ADV_PARTICLES_H

#include <string>
#include <list>

#include "Vector.h"

class Object2D;
class AdvDocument;

class ParticleEngine{
  struct Particle{
    Object2D* object;
    Vec2f position;
    Vec2f speed;
  };
public:
  ParticleEngine(AdvDocument* data);
  ~ParticleEngine();
  void update(unsigned time);
  void setParticleObject(const std::string& name, float initialRotation);
  void setDirection(Vec2f dir) {mDir = dir;}
  void setMaxParticles(int maxParticles) {mMaxParticles = maxParticles;}
  void setRotation(float angle) {mRotAngle = angle;}
  void setSpeedVariation(float variation) {mSpeedVariation = variation;}
  void activate(bool doit, bool immediately);
protected:
  void addParticle();
  AdvDocument* mData;
  Object2D* mParticleObject;
  Vec2f mDir;
  unsigned mMaxParticles;
  float mRotAngle;
  float mSpeedVariation;
  bool mEnabled;
  std::list<Particle> mParticles;
  unsigned mEmissionInterval;
  unsigned mTimeCount;
};

#endif
