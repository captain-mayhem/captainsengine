#include "Sound.h"

#include <iostream>
#include <cmath>
#include <system/allocation.h>
#include <system/engine.h>
#include <audio/audio.h>
#include <audio/audioeffect.h>

#include "EFXeffects.h"
#include "AdvDoc.h"
#include "Engine.h"
#include "SoundPlayer.h"
#include "SwfPlayer.h"

using namespace adv;

TR_CHANNEL(ADV_SOUND_ENGINE);

SoundEngine* SoundEngine::mInstance = nullptr;

namespace{

//! the EAX reverb presets of setEAXEffect (EFXeffects.h)
struct EAXPreset{
  const char* name;
  const EFXREVERBPROPERTIES* properties;
};

const EAXPreset EAX_PRESETS[] = {
  {"paddedcell", &REVERB_PRESET_PADDEDCELL},
  {"livingroom", &REVERB_PRESET_LIVINGROOM},
  {"cave", &REVERB_PRESET_CAVE},
  {"carpetedhallway", &REVERB_PRESET_CARPETTEDHALLWAY},
  {"alley", &REVERB_PRESET_ALLEY},
  {"mountains", &REVERB_PRESET_MOUNTAINS},
  {"parkinglot", &REVERB_PRESET_PARKINGLOT},
  {"drugged", &REVERB_PRESET_DRUGGED},
  {"concerthall", &REVERB_PRESET_CONCERTHALL},
  {"room", &REVERB_PRESET_ROOM},
  {"stoneroom", &REVERB_PRESET_STONEROOM},
  {"arena", &REVERB_PRESET_ARENA},
  {"hallway", &REVERB_PRESET_HALLWAY},
  {"forest", &REVERB_PRESET_FOREST},
  {"quarry", &REVERB_PRESET_QUARRY},
  {"sewerpipe", &REVERB_PRESET_SEWERPIPE},
  {"dizzy", &REVERB_PRESET_DIZZY},
  {"bathroom", &REVERB_PRESET_BATHROOM},
  {"auditorium", &REVERB_PRESET_AUDITORIUM},
  {"hangar", &REVERB_PRESET_HANGAR},
  {"stonecorridor", &REVERB_PRESET_STONECORRIDOR},
  {"city", &REVERB_PRESET_CITY},
  {"plain", &REVERB_PRESET_PLAIN},
  {"underwater", &REVERB_PRESET_UNDERWATER},
  {"psychotic", &REVERB_PRESET_PSYCHOTIC},
};

//! the engine's reverb has EFX's EAX reverb parameters
CGE::ReverbParams toReverb(const EFXREVERBPROPERTIES& p){
  CGE::ReverbParams r;
  r.density = p.flDensity;
  r.diffusion = p.flDiffusion;
  r.gain = p.flGain;
  r.gainHF = p.flGainHF;
  r.decayTime = p.flDecayTime;
  r.decayHFRatio = p.flDecayHFRatio;
  r.reflectionsGain = p.flReflectionsGain;
  r.reflectionsDelay = p.flReflectionsDelay;
  r.lateReverbGain = p.flLateReverbGain;
  r.lateReverbDelay = p.flLateReverbDelay;
  r.echoTime = p.flEchoTime;
  r.echoDepth = p.flEchoDepth;
  r.modulationTime = p.flModulationTime;
  r.modulationDepth = p.flModulationDepth;
  return r;
}

//! OpenAL's low-pass filters, which made the dry and the wet gains, took at most 1
float mixGain(float gain){
  return gain > 1.0f ? 1.0f : gain;
}

}

SoundEngine::SoundEngine() : mData(nullptr), mActiveMusic(nullptr), mActiveVideo(nullptr), mMusicVolume(1.0f), mSpeechVolume(1.0f), mCurrentEffect("none"), mIsDSPEffect(false), mFadingTime(300), mSpeedFactor(1.0){
  TR_USE(ADV_SOUND_ENGINE);
  CGE::Audio* audio = CGE::Engine::instance()->getAudio();
  if (audio)
    mEffect = audio->createEffect();
  if (!mEffect)
    TR_WARN("No sound: the engine's audio isn't available");
#ifndef ENGINE_SINGLE_THREADED
  mThread.setPriority(CGE::Thread::LOW);
  //start();
#endif
}

SoundEngine::~SoundEngine(){
  //stop();
  reset();
  //after the players sent through it
  mEffect.reset();
}

void SoundEngine::reset(){
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    delete iter->second;
  }
  mActiveSounds.clear();
  delete mActiveMusic;
  mActiveMusic = nullptr;
  delete mActiveVideo;
  mActiveVideo = nullptr;
  mMusicVolume = 1.0f;
  mSpeechVolume = 1.0f;
  mCurrentEffect = "none";
  mIsDSPEffect = false;
  mFadingTime = 300;
  mSpeedFactor = 1.0f;
  setMasterVolume(1.0f);
  if (mEffect){
    mEffect->setNone();
    mEffect->setMix(1, 1);
  }
}

void SoundEngine::setEAXEffect(const std::string& effect){
  if (effect == "off" || effect == "none"){
    mCurrentEffect = "none";
    mIsDSPEffect = false;
    //the players sent through the bus play dry from now on
    if (mEffect)
      mEffect->setNone();
    return;
  }
  const EFXREVERBPROPERTIES* preset = nullptr;
  for (const EAXPreset& eax : EAX_PRESETS){
    if (effect == eax.name)
      preset = eax.properties;
  }
  if (!preset){
    TR_USE(ADV_SOUND_ENGINE);
    TR_BREAK("Unknown effect %s", effect.c_str());
    return;
  }
  mCurrentEffect = effect;
  mIsDSPEffect = false;
  if (mEffect){
    mEffect->setReverb(toReverb(*preset));
    mEffect->setMix(1, 1);
  }
}

bool SoundEngine::setDSPEffect(const std::string& effect){
  TR_USE(ADV_SOUND_ENGINE);
  if (effect == "off" || effect == "none"){
    mCurrentEffect = "none";
    mIsDSPEffect = true;
    if (mEffect)
      mEffect->setNone();
    return true;
  }
  std::map<std::string, DSPEffect>::const_iterator iter = Engine::instance()->getSettings()->dspeffects.find(effect);
  if (iter == Engine::instance()->getSettings()->dspeffects.end()){
    TR_BREAK("unknown dsp effect %s", effect.c_str());
    return false;
  }
  //the StoryDesigner's parameters as the OpenAL build mapped them onto
  //OpenAL's effects, whose parameters the engine's effects take
  const DSPEffect& eff = iter->second;
  if (mEffect){
    switch (eff.type){
    case DSPEffect::REVERB:{
      //OpenAL's standard reverb: the defaults of ReverbParams, sized by the room size
      CGE::ReverbParams reverb;
      float rsize = eff.params[2] / 1000.0f;
      reverb.decayTime = rsize*(20.0f - 0.1f) + 0.1f;
      reverb.reflectionsDelay = rsize*0.3f;
      reverb.lateReverbDelay = rsize*0.1f;
      reverb.density = eff.params[3] / 1000.0f;
      reverb.diffusion = eff.params[4] / 1000.0f;
      mEffect->setReverb(reverb);
      mEffect->setMix(mixGain(eff.params[0] / 1000.0f), mixGain(eff.params[1] / 1000.0f));
      break;
    }
    case DSPEffect::ECHO:{
      CGE::EchoParams echo;
      echo.damping = 0;
      echo.feedback = (std::min)(std::abs(eff.params[2]) / 1000.f, 1.0f);
      echo.delay = (std::min)(eff.params[3] / 1000.f, 0.207f);
      echo.lrDelay = (std::min)(eff.params[3] / 1000.f, 0.404f);
      echo.spread = eff.params[2] < 0 ? -1.0f : 1.0f;
      mEffect->setEcho(echo);
      mEffect->setMix(mixGain(std::abs(eff.params[0]) / 1000.0f), mixGain((std::abs(eff.params[1]) / 1000.0f)*0.1f + 0.9f));
      break;
    }
    case DSPEffect::CHORUS:{
      CGE::ChorusParams chorus;
      chorus.delay = (std::min)(eff.params[4] / 400.0f * 0.016f, 0.016f);
      chorus.rate = (std::min)(eff.params[5] / 400.0f * 10.0f, 10.0f);
      chorus.feedback = 0.75f;
      chorus.depth = 0.5f;
      mEffect->setChorus(chorus);
      mEffect->setMix(mixGain(eff.params[0] / 1000.0f), mixGain(std::abs(eff.params[1]) / 1000.0f));
      break;
    }
    case DSPEffect::DISTORTION:{
      CGE::DistortionParams distortion;
      distortion.gain = (std::min)(eff.params[3] / 1000.f, 1.0f);
      distortion.edge = (std::min)(eff.params[4] / 300.f, 1.0f);
      distortion.lowpassCutoff = 24000;
      mEffect->setDistortion(distortion);
      mEffect->setMix(mixGain(std::abs(eff.params[0]) / 1000.0f * 2), mixGain(std::abs(eff.params[1]) / 1000.0f * 2));
      break;
    }
    case DSPEffect::PHASER:{
      //OpenAL's flanger
      CGE::ChorusParams flanger = CGE::ChorusParams::flanger();
      flanger.feedback = (std::min)(eff.params[2] / 1000.0f, 1.0f);
      flanger.rate = (std::min)(eff.params[3] / 1000.0f, 10.0f);
      flanger.depth = (std::min)(eff.params[4] / 10000.0f, 1.0f);
      flanger.phase = 180;
      flanger.waveform = CGE::ChorusParams::SINE;
      mEffect->setChorus(flanger);
      mEffect->setMix(mixGain(std::abs(eff.params[0]) / 1000.0f), mixGain(std::abs(eff.params[1]) / 1000.0f));
      break;
    }
    };
  }
  mCurrentEffect = effect;
  mIsDSPEffect = true;
  return true;
}

SoundPlayer* SoundEngine::getSound(const std::string& name, int flags){
  return getSound(name, isEffectEnabled(), flags);
}

SoundPlayer* SoundEngine::getSound(const std::string& name, const std::string& effect, int flags){
  std::string oldEffect = mCurrentEffect;
  setDSPEffect(effect);
  SoundPlayer* ret = getSound(name, isEffectEnabled(), flags);
  setDSPEffect(oldEffect);
  return ret;
}

SoundPlayer* SoundEngine::getSound(const std::string& name, bool effectEnabled, int flags){
  TR_USE(ADV_SOUND_ENGINE);
  mMutex.lock();
  if (!(flags & PLAYER_CREATE_ALWAYS)){
    std::multimap<std::string,SoundPlayer*>::iterator lowerit = mActiveSounds.find(name);
    if (lowerit != mActiveSounds.end() && lowerit->second != nullptr){
      SoundPlayer* ret = lowerit->second;
      mMutex.unlock();
      return ret;
    }
  }
  DataBuffer db;
  if (!mData->getSound(name, db)){
    mMutex.unlock();
    return nullptr;
  }
  SoundPlayer* plyr = createPlayer(name, db, effectEnabled);
  //TR_WARN("creating player %s %p", name.c_str(), plyr);
  if (plyr && !(flags & PLAYER_UNREALIZED) && !plyr->realize()){
    delete plyr;
    plyr = nullptr;
  }
  if (plyr && (flags & PLAYER_UNMANAGED))
    plyr->setAutoDelete(false);
  if (plyr)
    mActiveSounds.insert(std::make_pair(name, plyr));
  mMutex.unlock();
  return plyr;
}

SoundPlayer* SoundEngine::getMusic(const std::string& name){
  return getMusic(name, isEffectEnabled());
}

SoundPlayer* SoundEngine::getMusic(const std::string& name, bool effectEnabled){
  SoundPlayer* plyr = nullptr;
  if (name.empty() || (mActiveMusic != nullptr && mActiveMusic->getName() == name)){
    return mActiveMusic;
  }
  DataBuffer db;
  mData->getMusic(name, db);
  plyr = createPlayer(name, db, effectEnabled);
  if (plyr && !plyr->realize()){
    delete plyr;
    plyr = nullptr;
  }
  if (mActiveMusic){
    //crossfade
    mActiveMusic->fadeVolume(mMusicVolume, 0.0f, mFadingTime);
    std::multimap<std::string, SoundPlayer*>::iterator oldplayer = mActiveSounds.find(mActiveMusic->getName());
    if (oldplayer != mActiveSounds.end()){
      if (oldplayer->second){
        oldplayer->second->stop();
        delete oldplayer->second;
      }
      mActiveSounds.erase(oldplayer);
    }
    mActiveSounds.insert(std::make_pair(mActiveMusic->getName(), mActiveMusic));
  }
  mActiveMusic = plyr;
  if (plyr){
    plyr->fadeVolume(0.0f, mMusicVolume, mFadingTime);
  }
  return plyr;
}

VideoPlayer* SoundEngine::getMovie(const std::string& name, bool isSwf){
  VideoPlayer* plyr = nullptr;
  if (name.empty()){
    return mActiveVideo;
  }
  DataBuffer* db = new DataBuffer();
  mData->getMovie(name, *db);
  plyr = createVideoPlayer(name, *db, isSwf);
  if (plyr)
    plyr->realize();
  else
    delete db;
  if (mActiveVideo){
    mActiveVideo->stop();
    delete mActiveVideo;
  }
  mActiveVideo = plyr;
  return plyr;
}

SoundPlayer* SoundEngine::createPlayer(const std::string& name, const DataBuffer& db, bool effectEnabled){
  if (db.data == nullptr || db.length == 0 || !mEffect){
    return nullptr;
  }
  SoundPlayer* plyr = new SoundPlayer(name, effectEnabled);
  plyr->setStream(db);
  plyr->setSpeed(mSpeedFactor);
  return plyr;
}

VideoPlayer* SoundEngine::createVideoPlayer(const std::string& name, const DataBuffer& db, bool isSwf){
  TR_USE(ADV_SOUND_ENGINE);
  if (db.data == nullptr || db.length == 0){
    return nullptr;
  }
  if (isSwf){
    SwfPlayer* plyr = new SwfPlayer(name, &db);
    return plyr;
  }
  //videos (playavi, playvideo) were decoded with FFmpeg, which is gone;
  //their return with small decoders is planned (jewelhunter's PLAN.md)
  TR_WARN("Video %s isn't played: videos aren't supported", name.c_str());
  return nullptr;
}

void SoundEngine::update(unsigned time){
  run(time);
  if (mActiveVideo)
    mActiveVideo->render(time);
}

void SoundEngine::removeSpeaker(CharacterObject* chr){
  mMutex.lock();
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->getSpeaker() == chr){
      delete iter->second;
      iter->second = nullptr;
    }
  }
  mMutex.unlock();
}

void SoundEngine::setMusicVolume(float volume){
  mMusicVolume = volume;
  if (mActiveMusic != nullptr){
    if (!mActiveMusic->isFading())
      mActiveMusic->setVolume(volume);
  }
}

void SoundEngine::setSpeechVolume(float volume){
  mSpeechVolume = volume;
}

std::ostream& SoundEngine::save(std::ostream& out){
  out << mMusicVolume << " " << mSpeechVolume << "\n";
  out << mFadingTime << " " << mCurrentEffect << " " << mIsDSPEffect << "\n";
  out << mSpeedFactor << std::endl;
  CGE::Audio* audio = CGE::Engine::instance()->getAudio();
  float volume = audio ? audio->getMasterVolume() : 1.0f;
  out << volume << std::endl;
  if (mActiveMusic != nullptr){
    out << String(mActiveMusic->getName()) << " " << mActiveMusic->hasEffect();
  }
  else{
    out << String("none");
  }
  out << "\n";
  int loopcount = 0;
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->isLooping())
      ++loopcount;
  }
  out << loopcount;
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->isLooping()){
      out << " " << iter->second->getName() << " " << iter->second->hasEffect();
    }
  }
  out << "\n";
  return out;
}

std::istream& SoundEngine::load(std::istream& in, int version){
  //clear previous sounds
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->hasAutoDeletion()){
      delete iter->second;
      iter->second = nullptr;
    }
  }
  //mActiveSounds.clear(); //that does not work anymore with unmanaged sounds. Let the sound garbage collector do the work
  delete mActiveMusic;
  mActiveMusic = nullptr;

  in >> mMusicVolume >> mSpeechVolume;
  in >> mFadingTime >> mCurrentEffect;
  if (version >= 2)
    in >> mIsDSPEffect;
  else
    mIsDSPEffect = false;
  if (mIsDSPEffect)
    setDSPEffect(mCurrentEffect);
  else
    setEAXEffect(mCurrentEffect);
  in >> mSpeedFactor;
  float volume = 1.0f;
  if (version >= 2)
    in >> volume;
  setMasterVolume(volume);
  std::string music;
  if (version >= 2){
    String m;
    in >> m;
    music = m;
  }
  else
    in >> music;
  if (music != "none"){
    bool effect;
    in >> effect;
    SoundPlayer* mp = getMusic(music, effect);
    if (mp)
      mp->play(true);
  }
  int loopcount;
  in >> loopcount;
  for (int i = 0; i < loopcount; ++i){
    std::string sound;
    bool effect;
    in >> sound >> effect;
    SoundPlayer* sp = getSound(sound, effect, PLAYER_DEFAULT);
    if (sp)
      sp->play(true);
  }
  return in;
}

void SoundEngine::removeSoundPlayer(SoundPlayer* plyr){
  TR_USE(ADV_SOUND_ENGINE);
  mMutex.lock();
  std::multimap<std::string, SoundPlayer*>::iterator lower = mActiveSounds.lower_bound(plyr->getName());
  std::multimap<std::string, SoundPlayer*>::iterator upper = mActiveSounds.upper_bound(plyr->getName());
  while(lower != upper){
    if (lower->second == plyr){
      mActiveSounds.erase(lower);
      break;
    }
    ++lower;
  }
  mMutex.unlock();
  plyr->stop();
  //TR_WARN("deleting player %p", plyr);
  delete plyr;
}

void SoundEngine::setSpeedFactor(float speed){
  mMutex.lock();
  mSpeedFactor = speed;
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second == nullptr)
      continue;
    iter->second->setSpeed(speed);
  }
  if (mActiveMusic != nullptr)
    mActiveMusic->setSpeed(speed);
  mMutex.unlock();
}

bool SoundEngine::run(){
  static int64 lastTime = (int64)(getTime() * 1000);
  int64 now = (int64)(getTime() * 1000);
  int time = now - lastTime;
  lastTime = now;
  return run(time);
}

bool SoundEngine::run(unsigned time){
  mMutex.lock();
  std::multimap<std::string, SoundPlayer*>::iterator garbage = mActiveSounds.end();
  for (std::multimap<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && !iter->second->update(time)){
      delete iter->second;
      iter->second = nullptr;
    }
    if (iter->second == nullptr)
      garbage = iter;
  }
  if (garbage != mActiveSounds.end()){ //a little cleanup to get rid of old sounds
    mActiveSounds.erase(garbage);
    garbage = mActiveSounds.end();
  }
  if (mActiveMusic && !mActiveMusic->update(time)){
    delete mActiveMusic;
    mActiveMusic = nullptr;
  }
  if (mActiveVideo && !mActiveVideo->update(time)){
    delete mActiveVideo;
    mActiveVideo = nullptr;
  }
  mMutex.unlock();
  //CGE::Thread::sleep(10);
  return true;
}

void SoundEngine::setMasterVolume(float volume){
  CGE::Audio* audio = CGE::Engine::instance()->getAudio();
  if (audio)
    audio->setMasterVolume(volume);
}
