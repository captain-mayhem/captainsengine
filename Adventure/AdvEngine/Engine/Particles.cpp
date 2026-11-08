#include "Particles.h"

#include <cstdlib>
#include <cmath>

#include "AdvDoc.h"
#include "EngineObjects.h"
#include "Engine.h"
#include <system/allocation.h>

using namespace std;
using namespace adv;

namespace{

//! the script's speeds and rotations are per 40 ms
const float STEPS_PER_SECOND = 25.0f;

}

ParticleEngine::ParticleEngine(AdvDocument* data) : mData(data), mParticleObject(NULL), mDir(0,1),
mMaxParticles(200), mRotAngle(0), mSpeedVariation(0), mEnabled(false), mSystem(1 << 20), mEmissionInterval(20),
mTimeCount(0), mParticleDepth(DEPTH_PARTICLES_FRONT){
  //each particle's object goes with it
  mSystem.onRemove = [](CGE::Gfx::Particle& particle){
    delete objectOf(particle);
  };
  //the main room's barriers stop them
  mSystem.kill = [](const CGE::Gfx::Particle& particle){
    RoomObject* room = Engine::instance()->getRoom("");
    return room && Engine::instance()->areFXShapesEnabled() && room->hitsBarriers(particle.position, objectOf(particle)->getSize());
  };
}

ParticleEngine::~ParticleEngine(){
  mSystem.clear();
  delete mParticleObject;
}

void ParticleEngine::setParticleObject(const std::string& name, float initialRotation){
  Object* obj = mData->getObject(name);
  mSystem.clear();
  delete mParticleObject;
  mParticleObject = new Object2D(1, Vec2i(100,100), obj->size, obj->name);
  for (unsigned j = 0; j < obj->states.size(); ++j){
		Animation* anim = new Animation(obj->states[j].frames, obj->states[j].fps, mParticleDepth, obj->size);
    mParticleObject->addAnimation(anim);
  }
  mParticleObject->setRotation(initialRotation);
  mParticleObject->realize();
  mEmissionInterval = 20;//max(mParticleObject->getSize().x, mParticleObject->getSize().y);
  mEmissionInterval = (unsigned)(20*mEmissionInterval/mDir.length());
  return;
}

void ParticleEngine::activate(bool doit, bool immediately){
  mEnabled = doit;
  if (immediately){
    if (doit){
      for (int i = 0; i < 100; ++i){
        update(50, false);
      }
    }
    else{
      mSystem.clear();
    }
  }
}

void ParticleEngine::update(unsigned time, bool render){
  //the particles stay where they are in the room while it scrolls; the new
  //ones come in where the screen is now
  RoomObject* room = Engine::instance()->getRoom("");
  if (room){
    Vec2i scroll = room->getScrollOffset();
    mSystem.move(Vec2f(scroll-mScroll));
    mScroll = scroll;
  }
  mTimeCount += time;
  while (mTimeCount > mEmissionInterval){
    addParticle();
    mTimeCount -= mEmissionInterval;
  }
  //they are gone once they are off the screen and move away from it
  if (mParticleObject){
    Vec2i size = mParticleObject->getSize();
    int offset = max(size.x, size.y);
    int sizediff = offset - min(size.x, size.y);
    const Vec2i& resolution = Engine::instance()->getSettings()->resolution;
    mSystem.setBounds(Vec2f((float)-offset, (float)-offset), Vec2f((float)(resolution.x+sizediff), (float)(resolution.y+sizediff)));
  }
  mSystem.update(time/1000.0f);
  for (CGE::Gfx::Particle& particle : mSystem.getParticles()){
    Object2D* object = objectOf(particle);
    object->getAnimation()->update(time);
    object->setPosition(Vec2i((int)particle.position.x, (int)particle.position.y));
    object->setRotation(particle.rotation);
    if (render)
      object->render();
  }
}

void ParticleEngine::addParticle(){
  if (!mEnabled || !mParticleObject || mSystem.getCount() > (int)mMaxParticles)
    return;
  //calculate start position
  int range = Engine::instance()->getSettings()->resolution.x+Engine::instance()->getSettings()->resolution.y;
  int random = rand() % range;
  CGE::Gfx::Particle p;
  bool xdisallowed = abs(mDir.y) < 0.001f;
  bool ydisallowed = abs(mDir.x) < 0.001f;
  int xoffset = max(mParticleObject->getSize().x, mParticleObject->getSize().y);
  int yoffset = xoffset;
  if ((!ydisallowed && random > Engine::instance()->getSettings()->resolution.x) || xdisallowed){
    if (xdisallowed && random < Engine::instance()->getSettings()->resolution.y)
      random += Engine::instance()->getSettings()->resolution.y;
    p.position.y = (float)(random - Engine::instance()->getSettings()->resolution.x - yoffset/2);
    p.position.x = mDir.x < 0 ? (float)Engine::instance()->getSettings()->resolution.x : (float)(-xoffset);
  }
  else{
    if (ydisallowed && random > Engine::instance()->getSettings()->resolution.x)
      random = random % Engine::instance()->getSettings()->resolution.x;
    p.position.x = (float)(random - xoffset/2);
    p.position.y = mDir.y < 0 ? (float)Engine::instance()->getSettings()->resolution.y : (float)(-yoffset);
  }
  //set object
  Object2D* object = mParticleObject->clone();
  int numStates = object->getNumDefinedStates();
  int state = rand()%numStates;
  object->setState(state+1);
  //calculate particle speed
  float speedvar = rand()/(float)RAND_MAX;
  speedvar *= 2.0f*mSpeedVariation;
  speedvar += 1.0f-mSpeedVariation;
  p.velocity = mDir*(speedvar*STEPS_PER_SECOND);
  //the object's rotation (radians), turning
  p.rotation = object->getRotation();
  p.spin = mRotAngle*STEPS_PER_SECOND;
  p.user = (uintptr_t)object;
  if (!mSystem.add(p))
    delete object;
}

std::ostream& ParticleEngine::save(std::ostream& out){
  out << mDir.x << " " << mDir.y << " " << mMaxParticles << " " << mRotAngle << " " << mSpeedVariation << " " << mEnabled << " " << mParticleDepth << "\n";
  if (mParticleObject != NULL){
    out << mParticleObject->getName() << " " << mParticleObject->getRotation();
  }
  else{
    out << "none";
  }
  out << "\n";
  return out;
}

std::istream& ParticleEngine::load(std::istream& in){
  in >> mDir.x >> mDir.y >> mMaxParticles >> mRotAngle >> mSpeedVariation >> mEnabled >> mParticleDepth;
  std::string name;
  in >> name;
  if (name != "none"){
    float rot;
    in >> rot;
    setParticleObject(name, rot);
  }
  else{
    name = "";
  }
  return in;
}

void ParticleEngine::setDepth(int depth){
  mParticleDepth = depth;
  if (mParticleObject)
    mParticleObject->setDepth(depth);
}

bool ParticleEngine::Barrier::isHit(const Vec2f& position, const Vec2i& size, const Vec2f& offset){
  return intersectsLine(points[0], points[1], position-offset, size)
    || intersectsLine(points[1], points[2], position-offset, size)
    || intersectsLine(points[2], points[3], position-offset, size)
    || intersectsLine(points[3], points[0], position-offset, size);
}

bool ParticleEngine::Barrier::intersectsLine(Vec2f lineStart, Vec2f lineEnd, Vec2f rectPos, Vec2i size){
  int out1, out2;
  if ((out2 = outcode(lineEnd, rectPos, size)) == 0)
    return true;
  while((out1 = outcode(lineStart, rectPos, size)) != 0){
    if ((out1 & out2) != 0)
      return false;
    if ((out1 & (OUT_LEFT | OUT_RIGHT)) != 0){
      float x = (float)rectPos.x;
      if ((out1 & OUT_RIGHT) != 0)
        x += size.x;
      lineStart.y = lineStart.y + (x - lineStart.x) * (lineEnd.y - lineStart.y) / (lineEnd.x - lineStart.x);
      lineStart.x = x;
    }
    else{
      float y = (float)rectPos.y;
      if ((out1 & OUT_BOTTOM) != 0)
        y += size.y;
      lineStart.x = lineStart.x + (y - lineStart.y) * (lineEnd.x - lineStart.x) / (lineEnd.y - lineStart.y);
      lineStart.y = y;
    }
  }
  return true;
}

int ParticleEngine::Barrier::outcode(Vec2f point, Vec2f rectPos, Vec2i size){
  int out = 0;
  if (size.x <= 0)
    out |= OUT_LEFT | OUT_RIGHT;
  else if (point.x < rectPos.x)
    out |= OUT_LEFT;
  else if (point.x > rectPos.x + size.x)
    out |= OUT_RIGHT;
  if (size.y <= 0)
    out |= OUT_TOP | OUT_BOTTOM;
  else if (point.y < rectPos.y)
    out |= OUT_TOP;
  else if (point.y > rectPos.y + size.y)
    out |= OUT_BOTTOM;
  return out;
}
