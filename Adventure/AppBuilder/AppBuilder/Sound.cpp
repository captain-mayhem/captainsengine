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
  FILE* f = fopen(filename.c_str(), "wb");
  fwrite(db.data, 1, db.length, f);
  fclose(f);
  //ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  //SoundPlayer* plyr = new SimpleSoundPlayer(buffer);
  SoundPlayer* plyr = new StreamSoundPlayer(filename);
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
  alSourcei(mSource, AL_SOURCE_RELATIVE, AL_TRUE);
  alSourcei(mSource, AL_ROLLOFF_FACTOR, 0);
  alGenBuffers(3, mBuffers);
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
    mDecodeBuffer.used = 0;
    mDataBuffer.length = 0;
    mDataBuffer.data = NULL;
    mDataBuffer.used = 0;
    mALBuffer.length = BUFFER_SIZE;
    mALBuffer.data = new char[mALBuffer.length];
    mALBuffer.used = 0;
    if (mCodecContext->channels == 1)
      mPCMFormat = AL_FORMAT_MONO16;
    if (mCodecContext->channels == 2)
      mPCMFormat = AL_FORMAT_STEREO16;
    if (alIsExtensionPresent("AL_EXT_MCFORMATS")){
      if (mCodecContext->channels == 4)
        mPCMFormat = alGetEnumValue("AL_FORMAT_QUAD16");
      if (mCodecContext->channels == 6)
        mPCMFormat = alGetEnumValue("AL_FORMAT_51CHN16");
    }
    if (mPCMFormat == 0)
      continue;
  }
}

StreamSoundPlayer::~StreamSoundPlayer(){
  alDeleteBuffers(3, mBuffers);
  av_close_input_file(mFormat);
  remove(mFilename.c_str());
}

unsigned StreamSoundPlayer::decode(){
  if (!mCodecContext)
    return 0;
  unsigned decoded = 0;
  char* ptr = mALBuffer.data;
  while (decoded < mALBuffer.length){
    if (mDecodeBuffer.used > 0){
      unsigned remain = mALBuffer.length-decoded;
      if (remain > mDecodeBuffer.used)
        remain = mDecodeBuffer.used;
      memcpy(ptr, mDecodeBuffer.data, remain);
      ptr += remain;
      decoded += remain;
      if (remain < mDecodeBuffer.used){
        memmove(mDecodeBuffer.data, mDecodeBuffer.data+remain, mDecodeBuffer.used-remain);
      }
      mDecodeBuffer.used -= remain;
    }
    if (mDecodeBuffer.used == 0){
      unsigned insize = mDataBuffer.used;
      if (insize == 0){
        getNextPacket();
        if (insize == mDataBuffer.used)
          break;
        insize = mDataBuffer.used;
        memset(mDataBuffer.data+insize, 0, FF_INPUT_BUFFER_PADDING_SIZE);
      }
      int size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
      int length;
      while((length = avcodec_decode_audio2(mCodecContext, (int16_t*)mDecodeBuffer.data, &size, (uint8_t*)mDataBuffer.data, insize)) == 0){
        if (size > 0)
          break;
        getNextPacket();
        if (insize == mDataBuffer.used)
          break;
        insize = mDataBuffer.used;
        memset(mDataBuffer.data+insize, 0, FF_INPUT_BUFFER_PADDING_SIZE);
      }
      if (length < 0)
        break;
      if (length > 0){
        unsigned remain = insize-length;
        if (remain)
          memmove(mDataBuffer.data, mDataBuffer.data+length, remain);
        mDataBuffer.used = remain;
      }
      mDecodeBuffer.used = size;
    }
  }
  mALBuffer.used = decoded;
  return decoded;
}

void StreamSoundPlayer::getNextPacket(){
  AVPacket packet;
  while(av_read_frame(mFormat, &packet) >= 0){
    if (packet.stream_index != 0)
      continue;
    unsigned idx = mDataBuffer.used;
    if (idx+packet.size > mDataBuffer.length){
      char* temp = new char[idx+packet.size+FF_INPUT_BUFFER_PADDING_SIZE];
      memcpy(temp, mDataBuffer.data, mDataBuffer.used);
      delete [] mDataBuffer.data;
      mDataBuffer.data = temp;
      mDataBuffer.length = idx+packet.size+FF_INPUT_BUFFER_PADDING_SIZE;
    }
    memcpy(mDataBuffer.data+idx, packet.data, packet.size);
    mDataBuffer.used += packet.size;
    av_free_packet(&packet);
    break;
  }
}

void StreamSoundPlayer::play(bool looping){
  unsigned bytes;
  for (int i = 0; i < 3; ++i){
    bytes = decode();
    if (bytes == 0)
      break;
    alBufferData(mBuffers[i], mPCMFormat, mALBuffer.data, bytes, mCodecContext->sample_rate);
    alSourceQueueBuffers(mSource, 1, &mBuffers[i]);
  }
  alSourcePlay(mSource);
}

void StreamSoundPlayer::stop(){

}

bool StreamSoundPlayer::update(){
  ALint processed;
  alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
  if (processed == 0){
    //all buffers full
    ALint state;
    alGetSourcei(mSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING)
      alSourcePlay(mSource);
    else{
      ALint offset;
      alGetSourcei(mSource, AL_SAMPLE_OFFSET, &offset);
    }
    return true;
  }
  for (ALint i = 0; i < processed; ++i){
    unsigned bytes = decode();
    if (bytes > 0){
      ALuint curBuf;
      alSourceUnqueueBuffers(mSource, 1, &curBuf);
      if (curBuf != 0){
        alBufferData(curBuf, mPCMFormat, mALBuffer.data, bytes, mCodecContext->sample_rate);
        alSourceQueueBuffers(mSource, 1, &curBuf);
      }
    }
  }
  ALint state;
  alGetSourcei(mSource, AL_SOURCE_STATE, &state);
  if (state != AL_PLAYING)
    alSourcePlay(mSource);
  return true;
}
