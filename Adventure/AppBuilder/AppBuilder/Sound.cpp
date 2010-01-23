#include "Sound.h"

SoundEngine* SoundEngine::mInstance = NULL;

SoundEngine::SoundEngine(){
  mDevice = alcOpenDevice(NULL);
  if (mDevice){
    mContext = alcCreateContext(mDevice, NULL);
    alcMakeContextCurrent(mContext);
  }
}

SoundEngine::~SoundEngine(){
  alcMakeContextCurrent(NULL);
  alcDestroyContext(mContext);
  alcCloseDevice(mDevice);
}

SoundPlayer::SoundPlayer(const std::string file){
}

SoundPlayer::~SoundPlayer(){

}