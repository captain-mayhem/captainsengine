#include "Sound.h"
#include <AL/alut.h>
#include "AdvDoc.h"

SoundEngine* SoundEngine::mInstance = NULL;

SoundEngine::SoundEngine() : mData(NULL){
  mDevice = alcOpenDevice(NULL);
  if (mDevice){
    mContext = alcCreateContext(mDevice, NULL);
    alcMakeContextCurrent(mContext);
  }
  //init codecs
  alutInitWithoutContext(NULL, NULL);
}

SoundEngine::~SoundEngine(){
  for (std::map<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    delete iter->second;
  }
  mActiveSounds.clear();
  alcMakeContextCurrent(NULL);
  alcDestroyContext(mContext);
  alcCloseDevice(mDevice);
  //exit codecs
  alutExit();
}

SoundPlayer* SoundEngine::getSound(const std::string& name){
  SoundPlayer* plyr = mActiveSounds[name];
  if (plyr)
    return plyr;
  DataBuffer db;
  mData->getSound(name, db);
  ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  plyr = new SoundPlayer(buffer);
  mActiveSounds[name] = plyr;
  return plyr;
}

void SoundEngine::update(){
  for (std::map<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && !iter->second->update()){
      delete iter->second;
      iter->second = NULL;
    }
  }
}

SoundPlayer::SoundPlayer(ALuint buffer) : mBuffer(buffer){
  alGenSources(1, &mSource);
  alSourcei(mSource, AL_BUFFER, buffer);
}

SoundPlayer::~SoundPlayer(){
  alDeleteSources(1, &mSource);
  alDeleteBuffers(1, &mBuffer);
}

void SoundPlayer::play(bool looping){
  if (looping)
    alSourcei(mSource, AL_LOOPING, AL_TRUE);
  alSourcePlay(mSource);
}

void SoundPlayer::stop(){
  alSourceStop(mSource);
}

bool SoundPlayer::update(){
  ALint state;
  alGetSourcei(mSource, AL_SOURCE_STATE, &state);
  if (state == AL_STOPPED)
    return false;
  return true;
}

void SoundPlayer::setVolume(float volume){
  alSourcef(mSource, AL_GAIN, volume);
}
