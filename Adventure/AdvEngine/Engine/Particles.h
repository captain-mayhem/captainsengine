#ifndef ADV_PARTICLES_H
#define ADV_PARTICLES_H

#include <string>
#include <cstring>

#include <renderer/particles.h>

#include "Vector.h"

namespace adv{

class Object2D;
class AdvDocument;

//! The script's particles (setparticles, startparticles, stopparticles)
/*! Copies of an object come in from the edges of the screen that the
    direction points away from and fly across it, turning, until they leave
    it or hit one of the room's barriers (its FX shapes). The engine's
    CGE::Gfx::ParticleSystem moves them; each particle's user data is its
    object, which plays its animation and is drawn at the particle's depth. */
class ParticleEngine{
public:
  //! a room's FX shape, which particles don't pass
  class Barrier{
  public:
    Barrier(const Vec2i p[4]){
      memcpy(points, p, 4*sizeof(Vec2i));
    }
    //! whether the box of the size at position (in the room, minus offset) touches the shape's outline
    bool isHit(const Vec2f& position, const Vec2i& size, const Vec2f& offset);
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
  //! time in ms; draws the particles if render
  void update(unsigned time, bool render=true);
  void setParticleObject(const std::string& name, float initialRotation);
  //! pixels per 40 ms
  void setDirection(Vec2f dir) {mDir = dir;}
  void setMaxParticles(int maxParticles) {mMaxParticles = maxParticles;}
  //! the object's rotation per 40 ms
  void setRotation(float angle) {mRotAngle = angle;}
  void setSpeedVariation(float variation) {mSpeedVariation = variation;}
  void setDepth(int depth);
  int getDepth() {return mParticleDepth;}
  void activate(bool doit, bool immediately);
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);
protected:
  void addParticle();
  static Object2D* objectOf(const CGE::Gfx::Particle& particle) {return (Object2D*)particle.user;}
  AdvDocument* mData;
  Object2D* mParticleObject;
  Vec2f mDir;
  unsigned mMaxParticles;
  float mRotAngle;
  float mSpeedVariation;
  bool mEnabled;
  CGE::Gfx::ParticleSystem mSystem;
  //! the main room's scroll offset at the last update
  Vec2i mScroll;
  unsigned mEmissionInterval;
  unsigned mTimeCount;
  int mParticleDepth;
};

}

#endif
