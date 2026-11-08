#include "particles.h"

#include <algorithm>
#include <cmath>

#include "spritebatch.h"
#include "textureatlas.h"

using namespace CGE;
using namespace CGE::Gfx;

namespace{

const float PI = 3.14159265358979f;

float mix(float a, float b, float t){
  return a + (b-a)*t;
}

}

float Particle::getProgress() const{
  if (life <= 0)
    return 0;
  float t = age/life;
  return t < 0 ? 0 : (t > 1 ? 1 : t);
}

float Particle::getFade() const{
  float t = getProgress();
  if (t <= fadeFrom)
    return 0;
  return fadeFrom >= 1 ? 1 : (t-fadeFrom)/(1-fadeFrom);
}

Color Particle::getColor() const{
  float t = getFade();
  return Color(mix(color.r, endColor.r, t), mix(color.g, endColor.g, t), mix(color.b, endColor.b, t),
    mix(color.a, endColor.a, t));
}

float Particle::getSize() const{
  return mix(size, endSize, getFade());
}

ParticleSystem::ParticleSystem(int capacity, uint32_t seed) : capacity_(capacity < 0 ? 0 : capacity),
  random_(seed == 0 ? 1 : seed){
  particles_.reserve(std::min(capacity_, 4096));
}

ParticleSystem::~ParticleSystem(){
  clear();
}

float ParticleSystem::random(float min, float max){
  //xorshift32
  random_ ^= random_ << 13;
  random_ ^= random_ >> 17;
  random_ ^= random_ << 5;
  return min + (max-min)*((random_ >> 8)/16777216.0f);
}

Particle* ParticleSystem::add(const Particle& particle){
  if ((int)particles_.size() >= capacity_)
    return nullptr;
  particles_.push_back(particle);
  return &particles_.back();
}

Particle ParticleSystem::make(const Emission& e, const Vec2f& at, float direction){
  Particle p;
  float angle = direction/180.0f*PI;
  Vec2f dir(std::cos(angle), std::sin(angle));
  float speed = random(e.speedMin, e.speedMax);
  p.velocity = dir*speed;
  p.drag = e.drag;
  p.position = at + dir*e.offset;
  if (e.scatter > 0){
    //evenly over the disc
    float a = random(0, 2*PI);
    float r = e.scatter*std::sqrt(random(0, 1));
    p.position = p.position + Vec2f(std::cos(a)*r, std::sin(a)*r);
  }
  p.life = random(e.lifeMin, e.lifeMax);
  p.rotation = random(e.rotationMin, e.rotationMax) + (e.alongDirection ? direction : 0);
  p.spin = random(e.spinMin, e.spinMax);
  if (e.bothWays && random(0, 1) < 0.5f)
    p.spin = -p.spin;
  p.size = random(e.sizeMin, e.sizeMax);
  p.endSize = p.size*e.endSize;
  p.color = e.color;
  p.endColor = e.endColor;
  p.fadeFrom = e.fadeFrom;
  p.image = e.firstImage;
  if (e.images > 1)
    p.image += std::min(e.images-1, (int)random(0, (float)e.images));
  return p;
}

int ParticleSystem::emit(const Emission& e, const Vec2f& at, int count){
  int born = 0;
  for (int i = 0; i < count; ++i){
    float direction;
    if (e.even){
      //a full turn has no end that equals its start
      if (e.spread >= 360)
        direction = e.direction + i*e.spread/count;
      else
        direction = e.direction - e.spread/2 + e.spread*(i+0.5f)/count;
    }
    else{
      direction = e.direction + random(-e.spread/2, e.spread/2);
    }
    if (!add(make(e, at, direction)))
      break;
    ++born;
  }
  return born;
}

int ParticleSystem::emitAlong(const Emission& e, const Vec2f& from, const Vec2f& to, int count){
  int born = 0;
  for (int i = 0; i < count; ++i){
    float t = (i+0.5f)/count;
    born += emit(e, from + (to-from)*t, 1);
  }
  return born;
}

int ParticleSystem::trail(ParticleTrail& trail, const Emission& e, const Vec2f& to){
  if (!trail.started){
    trail.started = true;
    trail.last = to;
    trail.carried = 0;
    return 0;
  }
  Vec2f d = to - trail.last;
  float distance = d.length();
  int born = 0;
  if (trail.spacing > 0 && distance > 0){
    //the next particle is spacing after the last one, which was carried before trail.last
    float s = trail.spacing - trail.carried;
    float lastAt = -trail.carried;
    while (s <= distance){
      born += emit(e, trail.last + d*(s/distance), 1);
      lastAt = s;
      s += trail.spacing;
    }
    trail.carried = distance - lastAt;
  }
  trail.last = to;
  return born;
}

void ParticleSystem::update(float seconds){
  for (Particle& p : particles_){
    p.velocity = p.velocity + acceleration*seconds;
    if (p.drag > 0)
      p.velocity = p.velocity*std::exp(-p.drag*seconds);
    p.position = p.position + p.velocity*seconds;
    p.rotation += p.spin*seconds;
    p.age += seconds;
  }
  //the ones that stay keep their order
  auto gone = [&](const Particle& p){
    if (p.life > 0 && p.age >= p.life)
      return true;
    if (bounded_){
      if ((p.position.x < min_.x && p.velocity.x < 0) || (p.position.x > max_.x && p.velocity.x > 0) ||
        (p.position.y < min_.y && p.velocity.y < 0) || (p.position.y > max_.y && p.velocity.y > 0))
        return true;
    }
    return kill && kill(p);
  };
  std::vector<Particle>::iterator out = particles_.begin();
  for (std::vector<Particle>::iterator it = particles_.begin(); it != particles_.end(); ++it){
    if (gone(*it)){
      if (onRemove)
        onRemove(*it);
      continue;
    }
    if (out != it)
      *out = *it;
    ++out;
  }
  particles_.erase(out, particles_.end());
}

void ParticleSystem::move(const Vec2f& by){
  for (Particle& p : particles_)
    p.position = p.position + by;
}

void ParticleSystem::setBounds(const Vec2f& min, const Vec2f& max){
  bounded_ = true;
  min_ = min;
  max_ = max;
}

void ParticleSystem::clearBounds(){
  bounded_ = false;
}

void ParticleSystem::clear(){
  if (onRemove){
    for (Particle& p : particles_)
      onRemove(p);
  }
  particles_.clear();
}

int ParticleSystem::getCount() const{
  return (int)particles_.size();
}

int ParticleSystem::getCapacity() const{
  return capacity_;
}

const std::vector<Particle>& ParticleSystem::getParticles() const{
  return particles_;
}

std::vector<Particle>& ParticleSystem::getParticles(){
  return particles_;
}

void ParticleSystem::draw(SpriteBatch& batch, const AtlasRegion* const* images, int imageCount) const{
  if (imageCount <= 0)
    return;
  for (const Particle& p : particles_){
    const AtlasRegion* region = images[((p.image % imageCount) + imageCount) % imageCount];
    if (!region)
      continue;
    Sprite sprite = region->sprite(p.position.x, p.position.y);
    float size = p.getSize();
    sprite.scaleX = size;
    sprite.scaleY = size;
    sprite.rotation = p.rotation;
    sprite.color = p.getColor();
    batch.draw(*region, sprite);
  }
}

void ParticleSystem::draw(SpriteBatch& batch, const AtlasRegion* image) const{
  draw(batch, &image, 1);
}
