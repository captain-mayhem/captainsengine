#ifndef ADV_PARTICLES_H
#define ADV_PARTICLES_H

class Object2D;

class ParticleEngine{
public:
  ParticleEngine();
  ~ParticleEngine();
  void setParticleObject(Object2D* obj) {mParticleObject = obj;}
protected:
  Object2D* mParticleObject;
};

#endif
