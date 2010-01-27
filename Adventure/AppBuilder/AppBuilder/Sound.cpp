#include "Sound.h"
#include <AL/alut.h>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
};
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
  av_register_all();
  av_log_set_level(AV_LOG_ERROR);
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
  plyr = createPlayer(db);
  mActiveSounds[name] = plyr;
  return plyr;
}

SoundPlayer* SoundEngine::getMusic(const std::string& name){
  SoundPlayer* plyr = mActiveSounds[name];
  if (plyr)
    return plyr;
  DataBuffer db;
  mData->getMusic(name, db);
  plyr = createPlayer(db);
  mActiveSounds[name] = plyr;
  return plyr;
}

SoundPlayer* SoundEngine::createPlayer(const DataBuffer& db){
  char* tmp = tmpnam(NULL);
  std::string filename = mData->getProjectSettings()->savedir+"/tmp/"+tmp+db.name;
  FILE* f = fopen(filename.c_str(), "w");
  fwrite(db.data, 1, db.length, f);
  fclose(f);
  ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  SoundPlayer* plyr = new SimpleSoundPlayer(buffer);
  SoundPlayer* tmpply = new StreamSoundPlayer(filename);
  delete tmpply;
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

SoundPlayer::SoundPlayer(){
  alGenSources(1, &mSource);
}

SoundPlayer::~SoundPlayer(){
  alDeleteSources(1, &mSource);
}

void SoundPlayer::setVolume(float volume){
  alSourcef(mSource, AL_GAIN, volume);
}


SimpleSoundPlayer::SimpleSoundPlayer(ALuint buffer) : mBuffer(buffer){
  alSourcei(mSource, AL_BUFFER, buffer);
}

SimpleSoundPlayer::~SimpleSoundPlayer(){
  alDeleteBuffers(1, &mBuffer);
}

void SimpleSoundPlayer::play(bool looping){
  if (looping)
    alSourcei(mSource, AL_LOOPING, AL_TRUE);
  alSourcePlay(mSource);
}

void SimpleSoundPlayer::stop(){
  alSourceStop(mSource);
}

bool SimpleSoundPlayer::update(){
  ALint state;
  alGetSourcei(mSource, AL_SOURCE_STATE, &state);
  if (state == AL_STOPPED)
    return false;
  return true;
}

static const int BUFFER_SIZE = 19200;

StreamSoundPlayer::StreamSoundPlayer(const std::string& filename) : SoundPlayer(), mFilename(filename){
  alGenBuffers(4, mBuffers);
  if (av_open_input_file(&mFormat, filename.c_str(), NULL, 0, NULL) != 0)
    return;
  if (av_find_stream_info(mFormat) != 0)
    return;
  for (unsigned i = 0; i < mFormat->nb_streams; ++i){
    if (mFormat->streams[i]->codec->codec_type != CODEC_TYPE_AUDIO)
      continue;
    mCodecContext = mFormat->streams[i]->codec;
    mCodec = avcodec_find_decoder(mCodecContext->codec_id);
    if (!mCodec)
      continue;
    if (avcodec_open(mCodecContext, mCodec) < 0)
      continue;
    mDecodeBuffer.length = AVCODEC_MAX_AUDIO_FRAME_SIZE;
    mDecodeBuffer.data = new char[mDecodeBuffer.length];
  }
}

unsigned StreamSoundPlayer::decode(DataBuffer& db){
  unsigned decoded = 0;
  if (!mCodecContext)
    return 0;
  while (decoded < )
}

StreamSoundPlayer::~StreamSoundPlayer(){
  alDeleteBuffers(4, mBuffers);
  av_close_input_file(mFormat);
  remove(mFilename.c_str());
}

void StreamSoundPlayer::play(bool looping){

}

void StreamSoundPlayer::stop(){

}

bool StreamSoundPlayer::update(){
  return true;
}
