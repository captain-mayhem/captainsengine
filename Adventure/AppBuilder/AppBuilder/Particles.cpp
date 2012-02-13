#include "Particles.h"

#include <cstdlib>
#include <cmath>

#include "AdvDoc.h"
#include "EngineObjects.h"
#include "Engine.h"

using namespace std;
using namespace adv;

ParticleEngine::ParticleEngine(AdvDocument* data) : mData(data), mParticleObject(NULL), mDir(0,1), 
mMaxParticles(200), mRotAngle(0), mSpeedVariation(0), mEnabled(false), mEmissionInterval(20), 
mTimeCount(0), mParticleDepth(DEPTH_PARTICLES_TOP){
}

ParticleEngine::~ParticleEngine(){
  for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
    delete iter->object;
  }
  delete mParticleObject;
}

void ParticleEngine::setParticleObject(const std::string& name, float initialRotation){
  Object* obj = mData->getObject(name);
  for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
    delete iter->object;
  }
  mParticles.clear();
  delete mParticleObject;
  mParticleObject = new Object2D(1, Vec2i(100,100), obj->size, obj->name);
  for (unsigned j = 0; j < obj->states.size(); ++j){
    Animation* anim = new Animation(obj->states[j].frames, obj->states[j].fps, mParticleDepth);
    mParticleObject->addAnimation(anim);
  }
  mParticleObject->setRotation(initialRotation);
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
      for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
        delete iter->object;
      }
      mParticles.clear();
    }
  }
}

void ParticleEngine::update(unsigned time, bool render){
  mTimeCount += time;
  while (mTimeCount > mEmissionInterval){
    addParticle();
    mTimeCount -= mEmissionInterval;
  }
  for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
    RoomObject* room = Engine::instance()->getRoom("");
    iter->object->getAnimation()->update(time);
    iter->position += iter->speed*(float)time*0.025f;
    iter->object->setPosition(Vec2i((int)iter->position.x, (int)iter->position.y));
    if (room){
      iter->position += room->getScrollOffset()-iter->scrolloffset;
      iter->scrolloffset = room->getScrollOffset();
    }
    iter->object->setRotation(iter->object->getRotation()+mRotAngle*time*0.025f);
    //out of image
    int xoffset = max(iter->object->getSize().x, iter->object->getSize().y);
    int yoffset = xoffset;
    int sizediff = xoffset - min(iter->object->getSize().x, iter->object->getSize().y);
    if ((iter->object->getPosition().x < -xoffset && iter->speed.x < 0) || 
      (iter->object->getPosition().x > Engine::instance()->getSettings()->resolution.x+sizediff && iter->speed.x > 0)||
      (iter->object->getPosition().y < -yoffset && iter->speed.y < 0) ||
      (iter->object->getPosition().y > Engine::instance()->getSettings()->resolution.y+sizediff && iter->speed.y > 0)){
        delete iter->object;
        iter = mParticles.erase(iter);
        if (iter != mParticles.begin())
          --iter;
        if (iter == mParticles.end())
          break;
    }
    //barriers
    if (room && room->hitsBarriers(*iter)){
      delete iter->object;
      iter = mParticles.erase(iter);
      if (iter != mParticles.begin())
        --iter;
      if (iter == mParticles.end())
        break;
    }
    if (render)
      iter->object->render();
  }
}

void ParticleEngine::addParticle(){
  if (!mEnabled || mParticles.size() > mMaxParticles)
    return;
  //calculate start position
  int range = Engine::instance()->getSettings()->resolution.x+Engine::instance()->getSettings()->resolution.y;
  int random = rand() % range;
  Particle p;
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
  p.object = mParticleObject->clone();
  int numStates = p.object->getNumDefinedStates();
  int state = rand()%numStates;
  p.object->setState(state+1);
  //calculate particle speed
  float speedvar = rand()/(float)RAND_MAX;
  speedvar *= 2.0f*mSpeedVariation;
  speedvar += 1.0f-mSpeedVariation;
  p.speed = mDir*speedvar;
  p.scrolloffset = Vec2i();
  RoomObject* room = Engine::instance()->getRoom("");
  if (room)
    p.scrolloffset = room->getScrollOffset();
  mParticles.push_back(p);
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

bool ParticleEngine::Barrier::isHit(const Particle& particle, const Vec2f& offset){
  return intersectsLine(points[0], points[1], particle.position-offset, particle.object->getSize())
    || intersectsLine(points[1], points[2], particle.position-offset, particle.object->getSize())
    || intersectsLine(points[2], points[3], particle.position-offset, particle.object->getSize())
    || intersectsLine(points[3], points[0], particle.position-offset, particle.object->getSize());
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
