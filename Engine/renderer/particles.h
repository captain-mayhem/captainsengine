#ifndef CGE_PARTICLES_H
#define CGE_PARTICLES_H

#include <cge_export.h>

#include <cstdint>
#include <functional>
#include <vector>

#include "../math/vector.h"
#include "material.h"

namespace CGE{
namespace Gfx{

class SpriteBatch;
struct AtlasRegion;

//! one particle: where it is, how it moves, and how it looks over its life
struct CGE_API Particle{
  //! where it is drawn (its image's pivot)
  Vec2f position;
  //! pixels per second
  Vec2f velocity;
  //! how fast it slows down: its speed falls by the factor e^-drag each second (0: not at all)
  float drag = 0;
  //! seconds since it was born
  float age = 0;
  //! seconds it lives; 0 or less: until bounds or the kill test remove it
  float life = 0;
  //! degrees (draw), and degrees per second
  float rotation = 0;
  float spin = 0;
  //! its image's scale at birth and at the end of its life
  float size = 1;
  float endSize = 1;
  //! its colour at birth and at the end of its life (in between, linearly)
  Color color = Color(1, 1, 1, 1);
  Color endColor = Color(1, 1, 1, 1);
  //! the part of its life before its colour and size begin to change (0 to 1)
  float fadeFrom = 0;
  //! which of the images draw takes
  int image = 0;
  //! the owner's data (e.g. an object drawn for the particle)
  uintptr_t user = 0;

  //! how far through its life it is, 0 to 1 (0 for one without a life)
  float getProgress() const;
  //! how far its colour and size have changed, 0 to 1 (from fadeFrom on)
  float getFade() const;
  //! its colour and size now
  Color getColor() const;
  float getSize() const;
};

//! what emit gives the particles, each value picked between its min and max
struct Emission{
  //! seconds
  float lifeMin = 1;
  float lifeMax = 1;
  //! the direction in degrees from +x towards +y, and the spread around it
  /*! A direction is picked within direction-spread/2 to
      direction+spread/2; with even, a burst's directions are evenly spaced
      over the spread instead (spread 360: a ring). */
  float direction = 0;
  float spread = 360;
  bool even = false;
  //! pixels per second along the direction
  float speedMin = 0;
  float speedMax = 0;
  //! Particle::drag
  float drag = 0;
  //! born this far out along their direction (a ring's start radius)
  float offset = 0;
  //! born up to this far from the point, in any direction
  float scatter = 0;
  //! degrees; with alongDirection, the direction is added
  float rotationMin = 0;
  float rotationMax = 0;
  bool alongDirection = false;
  //! degrees per second; either way round when bothWays
  float spinMin = 0;
  float spinMax = 0;
  bool bothWays = false;
  //! the scale at birth, and the end's as a factor of it
  float sizeMin = 1;
  float sizeMax = 1;
  float endSize = 1;
  Color color = Color(1, 1, 1, 1);
  Color endColor = Color(1, 1, 1, 0);
  //! Particle::fadeFrom
  float fadeFrom = 0;
  //! the image is picked from firstImage to firstImage+images-1
  int firstImage = 0;
  int images = 1;
};

//! emits particles along a moving point's path, one every spacing pixels
/*! For trails behind things that move: ParticleSystem::trail is called with
    the point's new position whenever it moved. The spacing is kept across
    the calls, so many small moves emit as many particles as one big one. */
struct ParticleTrail{
  float spacing = 10;
  //! the last position, and the distance since the last particle
  Vec2f last;
  float carried = 0;
  bool started = false;
};

//! Particles for sparkles, explosions, trails and weather
/*! A pool of particles that move with their velocity, an acceleration
    (gravity) and their drag, spin, and change their colour and size over
    their life. emit and emitAlong make bursts from Emission's ranges; trail emits
    behind a moving point; add takes a particle as it is. update removes
    those whose life is over, those outside the bounds that move away from
    them, and those the kill test picks; onRemove hears of each. draw draws
    them through a SpriteBatch (each an instance of its quad, in the
    batch's current blend mode, e.g. Additive for light), with images from
    a texture atlas. The particles keep their order of birth.
    The random numbers are the system's own (a seed), so a run can be
    repeated. */
class CGE_API ParticleSystem{
public:
  //! capacity: the most particles at once; more aren't born
  explicit ParticleSystem(int capacity = 4096, uint32_t seed = 1);
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;

  //! pixels per second squared, for all particles (e.g. gravity)
  Vec2f acceleration;
  //! the particles to remove at the end of an update
  std::function<bool(const Particle&)> kill;
  //! hears of every particle removed (by update, clear or the destructor)
  std::function<void(Particle&)> onRemove;

  ~ParticleSystem();

  //! a particle as it is; nullptr when the system is full
  Particle* add(const Particle& particle);
  //! count particles at the point; returns how many were born
  int emit(const Emission& emission, const Vec2f& at, int count);
  //! count particles spread evenly along the line from from to to
  int emitAlong(const Emission& emission, const Vec2f& from, const Vec2f& to, int count);
  //! the trail's point moved to to: particles along its way, one every spacing
  int trail(ParticleTrail& trail, const Emission& emission, const Vec2f& to);

  //! the time passes: motion, spin, age, and the removals
  void update(float seconds);
  //! moves all particles (e.g. with a scrolling view)
  void move(const Vec2f& by);
  //! particles outside this rectangle that move away from it are removed
  void setBounds(const Vec2f& min, const Vec2f& max);
  void clearBounds();
  //! removes all particles
  void clear();

  int getCount() const;
  int getCapacity() const;
  const std::vector<Particle>& getParticles() const;
  std::vector<Particle>& getParticles();

  //! a random number from min to max (the system's own numbers)
  float random(float min, float max);

  //! the particles through the batch, each with images[image] (wrapping)
  /*! At the image's size times the particle's size, rotated around the
      image's pivot, in the particle's colour, in the batch's blend mode.
      Particles whose image is nullptr aren't drawn. */
  void draw(SpriteBatch& batch, const AtlasRegion* const* images, int imageCount) const;
  void draw(SpriteBatch& batch, const AtlasRegion* image) const;

private:
  Particle make(const Emission& emission, const Vec2f& at, float direction);

  std::vector<Particle> particles_;
  int capacity_;
  uint32_t random_;
  bool bounded_ = false;
  Vec2f min_;
  Vec2f max_;
};

}
}

#endif
