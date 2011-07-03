#include "Particles.h"

#include "AdvDoc.h"
#include "EngineObjects.h"

#include <cstdlib>

ParticleEngine::ParticleEngine(AdvDocument* data) : mData(data), mParticleObject(NULL), mDir(0,1), 
mMaxParticles(200), mRotAngle(0), mSpeedVariation(0), mEnabled(false){
}

ParticleEngine::~ParticleEngine(){
  delete mParticleObject;
}

void ParticleEngine::setParticleObject(const std::string& name){
  Object* obj = mData->getObject(name);
  delete mParticleObject;
  mParticleObject = new Object2D(1, Vec2i(100,100), obj->size, obj->name);
  for (unsigned j = 0; j < obj->states.size(); ++j){
    Animation* anim = new Animation(obj->states[j].frames, obj->states[j].fps, DEPTH_PARTICLES);
    mParticleObject->addAnimation(anim);
  }
  return;
}

void ParticleEngine::activate(bool doit, bool immediately){
  mEnabled = doit;
}

void ParticleEngine::update(unsigned time){
  if (!mEnabled)
    return;
  mParticleObject->render();
}
