#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <vector>

#include <renderer/drawcontext.h>
#include <renderer/nulldevice.h>
#include <renderer/particles.h>
#include <renderer/renderdevice.h>
#include <renderer/spritebatch.h>
#include <renderer/textureatlas.h>

using namespace CGE;
using namespace CGE::Gfx;

// ParticleSystem: emission, motion, life, the removals and drawing (on the
// Null device, which counts draw calls and instances).

namespace{

const float PI = 3.14159265358979f;

//! a burst that flies out at 100 px/s and lives 1 s
Emission burst(){
  Emission e;
  e.speedMin = e.speedMax = 100;
  return e;
}

float angleOf(const Vec2f& v){
  float a = std::atan2(v.y, v.x)*180/PI;
  return a < 0 ? a + 360 : a;
}

}

TEST(Particles, EmitBornWithTheEmissionsRanges){
  ParticleSystem system;
  Emission e;
  e.lifeMin = 0.5f;
  e.lifeMax = 1.5f;
  e.direction = 90;
  e.spread = 60;
  e.speedMin = 50;
  e.speedMax = 150;
  e.sizeMin = 0.5f;
  e.sizeMax = 2;
  e.endSize = 0.25f;
  e.drag = 2;
  e.fadeFrom = 0.3f;
  e.spinMin = 10;
  e.spinMax = 20;
  e.bothWays = true;
  e.firstImage = 2;
  e.images = 3;
  EXPECT_EQ(system.emit(e, Vec2f(10, 20), 500), 500);
  ASSERT_EQ(system.getCount(), 500);
  bool images[3] = {};
  bool negative = false;
  bool positive = false;
  for (const Particle& p : system.getParticles()){
    EXPECT_FLOAT_EQ(p.position.x, 10);
    EXPECT_FLOAT_EQ(p.position.y, 20);
    EXPECT_GE(p.life, 0.5f);
    EXPECT_LE(p.life, 1.5f);
    float speed = p.velocity.length();
    EXPECT_GE(speed, 50 - 0.01f);
    EXPECT_LE(speed, 150 + 0.01f);
    float a = angleOf(p.velocity);
    EXPECT_GE(a, 60 - 0.01f);
    EXPECT_LE(a, 120 + 0.01f);
    EXPECT_GE(p.size, 0.5f);
    EXPECT_LE(p.size, 2);
    EXPECT_FLOAT_EQ(p.endSize, p.size*0.25f);
    EXPECT_FLOAT_EQ(p.drag, 2);
    EXPECT_FLOAT_EQ(p.fadeFrom, 0.3f);
    EXPECT_GE(std::fabs(p.spin), 10);
    EXPECT_LE(std::fabs(p.spin), 20);
    negative |= p.spin < 0;
    positive |= p.spin > 0;
    ASSERT_GE(p.image, 2);
    ASSERT_LE(p.image, 4);
    images[p.image-2] = true;
  }
  EXPECT_TRUE(negative && positive);
  EXPECT_TRUE(images[0] && images[1] && images[2]);
}

TEST(Particles, EvenBurstIsARing){
  ParticleSystem system;
  Emission e = burst();
  e.even = true;
  e.offset = 30;
  e.direction = 10;
  e.alongDirection = true;
  system.emit(e, Vec2f(100, 100), 12);
  ASSERT_EQ(system.getCount(), 12);
  for (int i = 0; i < 12; ++i){
    const Particle& p = system.getParticles()[i];
    float expected = 10 + i*30.0f;
    EXPECT_NEAR(angleOf(p.velocity), expected, 0.01f);
    EXPECT_NEAR(p.rotation, expected, 0.01f);
    //born on the ring
    EXPECT_NEAR((p.position - Vec2f(100, 100)).length(), 30, 0.01f);
  }
  //an even spread of less than a turn: from its one end to its other
  ParticleSystem fan;
  e.direction = 0;
  e.spread = 90;
  fan.emit(e, Vec2f(), 3);
  EXPECT_NEAR(angleOf(fan.getParticles()[0].velocity), 360 - 30, 0.01f);
  EXPECT_NEAR(angleOf(fan.getParticles()[1].velocity), 0, 0.01f);
  EXPECT_NEAR(angleOf(fan.getParticles()[2].velocity), 30, 0.01f);
}

TEST(Particles, ScatterWithinTheRadius){
  ParticleSystem system;
  Emission e;
  e.scatter = 20;
  system.emit(e, Vec2f(5, 5), 300);
  float farthest = 0;
  for (const Particle& p : system.getParticles())
    farthest = std::max(farthest, (p.position - Vec2f(5, 5)).length());
  EXPECT_LE(farthest, 20 + 0.01f);
  EXPECT_GT(farthest, 15);
}

TEST(Particles, TheCapacityIsTheMost){
  ParticleSystem system(10);
  EXPECT_EQ(system.emit(burst(), Vec2f(), 8), 8);
  EXPECT_EQ(system.emit(burst(), Vec2f(), 8), 2);
  EXPECT_EQ(system.getCount(), 10);
  EXPECT_EQ(system.add(Particle()), nullptr);
  //space again once some are gone
  system.getParticles()[0].life = 0.1f;
  system.update(0.2f);
  EXPECT_NE(system.add(Particle()), nullptr);
}

TEST(Particles, MotionSpinAndAge){
  ParticleSystem system;
  Particle p;
  p.position = Vec2f(0, 0);
  p.velocity = Vec2f(10, 0);
  p.spin = 90;
  p.life = 10;
  system.add(p);
  system.update(0.5f);
  const Particle& q = system.getParticles()[0];
  EXPECT_FLOAT_EQ(q.position.x, 5);
  EXPECT_FLOAT_EQ(q.rotation, 45);
  EXPECT_FLOAT_EQ(q.age, 0.5f);
  //gravity
  system.acceleration = Vec2f(0, 100);
  system.update(1);
  EXPECT_FLOAT_EQ(system.getParticles()[0].velocity.y, 100);
  EXPECT_FLOAT_EQ(system.getParticles()[0].position.y, 100);
  //drag: e^-1 of the speed left after a second, in one step or many
  system.acceleration = Vec2f();
  system.getParticles()[0].drag = 1;
  Particle twin = system.getParticles()[0];
  system.update(1);
  EXPECT_NEAR(system.getParticles()[0].velocity.x, 10*std::exp(-1.0f), 0.0001f);
  ParticleSystem steps;
  steps.add(twin);
  for (int i = 0; i < 10; ++i)
    steps.update(0.1f);
  EXPECT_NEAR(steps.getParticles()[0].velocity.x, system.getParticles()[0].velocity.x, 0.0001f);
  //move
  Vec2f before = system.getParticles()[0].position;
  system.move(Vec2f(3, -4));
  EXPECT_FLOAT_EQ(system.getParticles()[0].position.x, before.x + 3);
  EXPECT_FLOAT_EQ(system.getParticles()[0].position.y, before.y - 4);
}

TEST(Particles, ColourAndSizeOverTheLife){
  Particle p;
  p.life = 2;
  p.size = 2;
  p.endSize = 0;
  p.color = Color(1, 0, 0, 1);
  p.endColor = Color(0, 0, 1, 0);
  EXPECT_FLOAT_EQ(p.getSize(), 2);
  p.age = 0.5f;
  EXPECT_FLOAT_EQ(p.getProgress(), 0.25f);
  EXPECT_FLOAT_EQ(p.getSize(), 1.5f);
  Color c = p.getColor();
  EXPECT_FLOAT_EQ(c.r, 0.75f);
  EXPECT_FLOAT_EQ(c.b, 0.25f);
  EXPECT_FLOAT_EQ(c.a, 0.75f);
  p.age = 5;
  EXPECT_FLOAT_EQ(p.getProgress(), 1);
  EXPECT_FLOAT_EQ(p.getSize(), 0);
  //fading from half its life: nothing before, all of it at the end
  p.fadeFrom = 0.5f;
  p.age = 0.9f;
  EXPECT_FLOAT_EQ(p.getSize(), 2);
  EXPECT_FLOAT_EQ(p.getColor().a, 1);
  p.age = 1.5f;
  EXPECT_FLOAT_EQ(p.getFade(), 0.5f);
  EXPECT_FLOAT_EQ(p.getSize(), 1);
  p.age = 2;
  EXPECT_FLOAT_EQ(p.getColor().a, 0);
  p.fadeFrom = 0;
  //without a life: as at birth
  p.life = 0;
  EXPECT_FLOAT_EQ(p.getSize(), 2);
  EXPECT_FLOAT_EQ(p.getColor().r, 1);
}

TEST(Particles, RemovedAtTheEndOfTheirLifeInOrder){
  ParticleSystem system;
  std::vector<uintptr_t> removed;
  system.onRemove = [&](Particle& p){ removed.push_back(p.user); };
  for (int i = 0; i < 6; ++i){
    Particle p;
    p.life = i % 2 ? 1.0f : 3.0f;
    p.user = i;
    system.add(p);
  }
  system.update(0.9f);
  EXPECT_EQ(system.getCount(), 6);
  system.update(0.2f);
  ASSERT_EQ(system.getCount(), 3);
  EXPECT_EQ(removed, (std::vector<uintptr_t>{1, 3, 5}));
  //the others keep their order
  EXPECT_EQ(system.getParticles()[0].user, 0u);
  EXPECT_EQ(system.getParticles()[1].user, 2u);
  EXPECT_EQ(system.getParticles()[2].user, 4u);
  //a particle without a life stays
  Particle forever;
  forever.user = 9;
  system.add(forever);
  system.update(100);
  ASSERT_EQ(system.getCount(), 1);
  EXPECT_EQ(system.getParticles()[0].user, 9u);
  //clear tells too
  system.clear();
  EXPECT_EQ(system.getCount(), 0);
  EXPECT_EQ(removed.back(), 9u);
}

TEST(Particles, BoundsRemoveThoseMovingAway){
  ParticleSystem system;
  system.setBounds(Vec2f(0, 0), Vec2f(100, 100));
  Particle in;
  in.position = Vec2f(-50, 50);
  in.velocity = Vec2f(10, 0);
  in.user = 1;
  Particle out = in;
  out.velocity = Vec2f(-10, 0);
  out.user = 2;
  Particle below = in;
  below.position = Vec2f(50, 150);
  below.velocity = Vec2f(0, 1);
  below.user = 3;
  Particle inside = in;
  inside.position = Vec2f(50, 50);
  inside.velocity = Vec2f(-1000, 0);
  inside.user = 4;
  system.add(in);
  system.add(out);
  system.add(below);
  system.add(inside);
  system.update(0.01f);
  //coming in from outside, and still inside
  ASSERT_EQ(system.getCount(), 2);
  EXPECT_EQ(system.getParticles()[0].user, 1u);
  EXPECT_EQ(system.getParticles()[1].user, 4u);
  //the fast one leaves at the left
  system.update(0.1f);
  ASSERT_EQ(system.getCount(), 1);
  system.clearBounds();
  system.getParticles()[0].velocity = Vec2f(-1000, 0);
  system.update(1);
  EXPECT_EQ(system.getCount(), 1);
}

TEST(Particles, KillTest){
  ParticleSystem system;
  system.kill = [](const Particle& p){ return p.position.x > 10; };
  int removed = 0;
  system.onRemove = [&](Particle&){ ++removed; };
  Particle p;
  p.velocity = Vec2f(10, 0);
  system.add(p);
  system.update(0.5f);
  EXPECT_EQ(system.getCount(), 1);
  system.update(1);
  EXPECT_EQ(system.getCount(), 0);
  EXPECT_EQ(removed, 1);
}

TEST(Particles, TrailOneEverySpacing){
  Emission e;
  e.lifeMin = e.lifeMax = 10;
  //one big move
  ParticleSystem big;
  ParticleTrail a;
  a.spacing = 10;
  EXPECT_EQ(big.trail(a, e, Vec2f(0, 0)), 0);
  EXPECT_EQ(big.trail(a, e, Vec2f(95, 0)), 9);
  //many small ones: the same particles
  ParticleSystem small;
  ParticleTrail b;
  b.spacing = 10;
  small.trail(b, e, Vec2f(0, 0));
  int born = 0;
  for (int i = 1; i <= 38; ++i)
    born += small.trail(b, e, Vec2f(i*2.5f, 0));
  EXPECT_EQ(born, 9);
  ASSERT_EQ(small.getCount(), big.getCount());
  for (int i = 0; i < big.getCount(); ++i){
    EXPECT_NEAR(big.getParticles()[i].position.x, 10.0f*(i+1), 0.001f);
    EXPECT_NEAR(small.getParticles()[i].position.x, 10.0f*(i+1), 0.001f);
  }
  //around a corner, the spacing counts along the path
  big.trail(a, e, Vec2f(95, 10));
  EXPECT_EQ(big.getCount(), 10);
  EXPECT_NEAR(big.getParticles()[9].position.x, 95, 0.001f);
  EXPECT_NEAR(big.getParticles()[9].position.y, 5, 0.001f);
}

TEST(Particles, EmitAlongALine){
  ParticleSystem system;
  system.emitAlong(Emission(), Vec2f(0, 0), Vec2f(40, 0), 4);
  ASSERT_EQ(system.getCount(), 4);
  for (int i = 0; i < 4; ++i)
    EXPECT_FLOAT_EQ(system.getParticles()[i].position.x, 5 + 10.0f*i);
}

TEST(Particles, TheSameSeedTheSameParticles){
  ParticleSystem a(100, 7);
  ParticleSystem b(100, 7);
  ParticleSystem c(100, 8);
  a.emit(burst(), Vec2f(), 5);
  b.emit(burst(), Vec2f(), 5);
  c.emit(burst(), Vec2f(), 5);
  bool differs = false;
  for (int i = 0; i < 5; ++i){
    EXPECT_FLOAT_EQ(a.getParticles()[i].velocity.x, b.getParticles()[i].velocity.x);
    differs |= a.getParticles()[i].velocity.x != c.getParticles()[i].velocity.x;
  }
  EXPECT_TRUE(differs);
}

TEST(Particles, DrawnAsInstancesInOneDraw){
  std::unique_ptr<RenderDevice> device = RenderDevice::create("Null");
  ASSERT_TRUE(device->init(NativeWindow(), DeviceDesc()));
  NullDevice* null = static_cast<NullDevice*>(device.get());
  {
    DrawContext ctx(*device);
    TextureDesc desc;
    desc.width = desc.height = 4;
    std::unique_ptr<Texture> texture = device->createTexture(desc);
    AtlasRegion spark;
    spark.texture = texture.get();
    spark.width = spark.height = spark.trimWidth = spark.trimHeight = 8;
    AtlasRegion star = spark;
    const AtlasRegion* images[3] = {&spark, &star, nullptr};
    ParticleSystem system;
    Emission e = burst();
    e.images = 3;
    system.emit(e, Vec2f(), 300);
    int drawn = 0;
    for (const Particle& p : system.getParticles())
      drawn += p.image != 2;
    SpriteBatch batch(ctx);
    ASSERT_TRUE(device->beginFrame());
    ASSERT_TRUE(device->beginPass(PassDesc()));
    batch.setBlend(BlendMode::Additive);
    system.draw(batch, images, 3);
    EXPECT_EQ(batch.getPending(), drawn);
    batch.flush();
    //one texture, one blend mode: one instanced draw
    EXPECT_EQ(null->getDrawCount(), 1);
    EXPECT_EQ(null->getInstanceCount(), drawn);
    device->endPass();
    device->endFrame();
  }
}
