#include "SoundPlayer.h"

#include <audio/audio.h>
#include <audio/audioplayer.h>

#include "Sound.h"
#include "Engine.h"

using namespace adv;

TR_CHANNEL(ADV_Sound_Player);

SoundPlayer::SoundPlayer(const std::string& name, bool effectEnabled) : mSpeaker(nullptr), mSuspensionScript(nullptr), mSpokenString(nullptr),
mName(name), mStartVolume(1.0f), mEndVolume(1.0f), mFadeDuration(0), mCurrTime(0), mEffectEnabled(effectEnabled), mSpeed(1.0f), mVolume(1.0f),
mLooping(false), mPlay(false), mAutoDelete(true){
  TR_USE(ADV_Sound_Player);
  TR_DEBUG("Creating sound player %s", name.c_str());
}

SoundPlayer::~SoundPlayer(){
  if (mSuspensionScript){
    mSuspensionScript->resume();
    ExecutionContext::unref(mSuspensionScript);
    mSuspensionScript = nullptr;
  }
  if (mSpeaker){
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(mSpeaker, false);
    mSpeaker->setTalking(false);
    mSpeaker = nullptr;
  }
  if (mSpokenString){
    Engine::instance()->getFontRenderer()->removeText((FontRenderer::String*)mSpokenString);
    mSpokenString = nullptr;
  }
}

void SoundPlayer::setStream(const DataBuffer& buffer){
  mFile.assign((const uint8_t*)buffer.data, (const uint8_t*)buffer.data+buffer.length);
}

bool SoundPlayer::realize(){
  TR_USE(ADV_Sound_Player);
  if (mPlayer)
    return true;
  CGE::Audio* audio = CGE::Engine::instance()->getAudio();
  if (!audio)
    return false;
  mPlayer = audio->createPlayer(std::move(mFile));
  mFile.clear();
  if (!mPlayer){
    TR_WARN("Sound %s can't be played", mName.c_str());
    return false;
  }
  if (mEffectEnabled)
    mPlayer->setEffect(SoundEngine::instance()->getEffect());
  mPlayer->setVolume(mVolume);
  mPlayer->setPitch(mSpeed);
  return true;
}

void SoundPlayer::play(bool looping){
  mLooping = looping;
  mPlay = true;
  if (mPlayer)
    mPlayer->play(looping);
}

void SoundPlayer::stop(){
  mLooping = false;
  if (mPlayer)
    mPlayer->stop();
}

void SoundPlayer::seek(int time){
  if (mPlayer)
    mPlayer->seek(time/1000.0);
}

void SoundPlayer::setVolume(float volume){
  mVolume = volume;
  if (mPlayer)
    mPlayer->setVolume(volume);
}

bool SoundPlayer::update(unsigned time){
  TR_USE(ADV_Sound_Player);
  TR_DETAIL("updating with time %i", time);
  if (!mPlay)
    return true;
  fadeUpdate(time);
  if (mPlayer && mPlayer->isPlaying())
    return true;
  //at its end, or stopped
  mPlay = false;
  return !mAutoDelete;
}

void SoundPlayer::setSuspensionScript(ExecutionContext* ctx){
  if (mSuspensionScript != nullptr){
    mSuspensionScript->reset(true,true);
    ExecutionContext::unref(mSuspensionScript);
  }
  ctx->ref();
  mSuspensionScript = ctx;
}

void SoundPlayer::fadeVolume(float from, float to, unsigned time){
  mStartVolume = from;
  mEndVolume = to;
  mFadeDuration = time;
  mCurrTime = 0;
  setVolume(mStartVolume);
}

bool SoundPlayer::fadeUpdate(unsigned time){
  if (mFadeDuration == 0)
    return true;
  mCurrTime += time;
  float factor = mCurrTime/(float)mFadeDuration;
  if (factor > 1.0f)
    factor = 1.0f;
  float volume = (1-factor)*mStartVolume + factor * mEndVolume;
  setVolume(volume);
  if (mCurrTime >= mFadeDuration){
    mFadeDuration = 0;
    if (mEndVolume == 0.0f)
      stop();
    return false;
  }
  return true;
}

void SoundPlayer::setSpeed(float factor){
  mSpeed = factor;
  if (mPlayer)
    mPlayer->setPitch(factor);
}
