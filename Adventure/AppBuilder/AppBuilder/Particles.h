#ifndef ADV_PARTICLES_H
#define ADV_PARTICLES_H

#include <string>
#include <list>
#include <cstring>

#include "Vector.h"

namespace adv{

class Object2D;
class AdvDocument;

class ParticleEngine{
public:
  struct Particle{
    Object2D* object;
    Vec2f position;
    Vec2f speed;
    Vec2i scrolloffset;
  };
  class Barrier{
  public:
    Barrier(const Vec2i p[4]){
      memcpy(points, p, 4*sizeof(Vec2i));
    }
    bool isHit(const Particle& particle, const Vec2f& offset);
  private:
    enum Outcode{
      OUT_LEFT = 1,
      OUT_TOP = 2,
      OUT_RIGHT = 4,
      OUT_BOTTOM = 8,
    };
    static int outcode(Vec2f point, Vec2f rectPos, Vec2i size);
    static bool intersectsLine(Vec2f lineStart, Vec2f lineEnd, Vec2f rectPos, Vec2i size);
    Vec2i points[4];
  };
  ParticleEngine(AdvDocument* data);
  ~ParticleEngine();
  void update(unsigned time, bool render=true);
  void setParticleObject(const std::string& name, float initialRotation);
  void setDirection(Vec2f dir) {mDir = dir;}
  void setMaxParticles(int maxParticles) {mMaxParticles = maxParticles;}
  void setRotation(float angle) {mRotAngle = angle;}
  void setSpeedVariation(float variation) {mSpeedVariation = variation;}
  void setDepth(int depth);
  void activate(bool doit, bool immediately);
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);
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
  int mParticleDepth;
};

}

#endif
