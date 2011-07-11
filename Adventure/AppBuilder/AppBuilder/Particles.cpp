#include "Particles.h"

#include <cstdlib>
#include <cmath>

#include "AdvDoc.h"
#include "EngineObjects.h"
#include "Engine.h"

ParticleEngine::ParticleEngine(AdvDocument* data) : mData(data), mParticleObject(NULL), mDir(0,1), 
mMaxParticles(200), mRotAngle(0), mSpeedVariation(0), mEnabled(false), mEmissionInterval(20), mTimeCount(0){
}

ParticleEngine::~ParticleEngine(){
  for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
    delete iter->object;
  }
  delete mParticleObject;
}

void ParticleEngine::setParticleObject(const std::string& name){
  Object* obj = mData->getObject(name);
  for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
    delete iter->object;
  }
  mParticles.clear();
  delete mParticleObject;
  mParticleObject = new Object2D(1, Vec2i(100,100), obj->size, obj->name);
  for (unsigned j = 0; j < obj->states.size(); ++j){
    Animation* anim = new Animation(obj->states[j].frames, obj->states[j].fps, DEPTH_PARTICLES);
    mParticleObject->addAnimation(anim);
  }
  mEmissionInterval = 20;//max(mParticleObject->getSize().x, mParticleObject->getSize().y);
  mEmissionInterval = (unsigned)(20*mEmissionInterval/mDir.length());
  return;
}

void ParticleEngine::activate(bool doit, bool immediately){
  if (immediately)
    DebugBreak();
  mEnabled = doit;
}

void ParticleEngine::update(unsigned time){
  mTimeCount += time;
  while (mTimeCount > mEmissionInterval){
    addParticle();
    mTimeCount -= mEmissionInterval;
  }
  for (std::list<Particle>::iterator iter = mParticles.begin(); iter != mParticles.end(); ++iter){
    iter->object->getAnimation()->update(time);
    iter->position += iter->speed*(float)time*0.025f;
    iter->object->setPosition(Vec2i((int)iter->position.x, (int)iter->position.y));
    iter->object->setRotation(iter->object->getRotation()+mRotAngle*time*0.025f);
    //out of image
    if (iter->object->getPosition().x < -iter->object->getSize().x || 
      iter->object->getPosition().x > Engine::instance()->getSettings()->resolution.x ||
      iter->object->getPosition().y < -iter->object->getSize().y ||
      iter->object->getPosition().y > Engine::instance()->getSettings()->resolution.y){
        delete iter->object;
        iter = mParticles.erase(iter);
        if (iter != mParticles.begin())
          --iter;
        if (iter == mParticles.end())
          break;
    }
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
  if ((!ydisallowed && random > Engine::instance()->getSettings()->resolution.x) || xdisallowed){
    if (xdisallowed && random < Engine::instance()->getSettings()->resolution.y)
      random += Engine::instance()->getSettings()->resolution.y;
    p.position.y = (float)(random - Engine::instance()->getSettings()->resolution.x - mParticleObject->getSize().y/2);
    p.position.x = mDir.x < 0 ? (float)Engine::instance()->getSettings()->resolution.x : (float)(-mParticleObject->getSize().x);
  }
  else{
    if (ydisallowed && random > Engine::instance()->getSettings()->resolution.x)
      random = random % Engine::instance()->getSettings()->resolution.x;
    p.position.x = (float)(random - mParticleObject->getSize().x/2);
    p.position.y = mDir.y < 0 ? (float)Engine::instance()->getSettings()->resolution.y : (float)(-mParticleObject->getSize().y);
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
  mParticles.push_back(p);
}
