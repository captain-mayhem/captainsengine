#include "Sound.h"

#include <iostream>

#ifndef UINT64_C
#define UINT64_C(val) val##ui64
#endif

#ifndef DISABLE_SOUND
#include <AL/alut.h>
#ifdef UNIX
#include <AL/efx.h>
#else
#include <efx.h>
#include <efx-creative.h>
//#include <EFX-Util.h>
#endif
#include "EFXeffects.h"
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};
#endif
#include "AdvDoc.h"
#include "Engine.h"

TR_CHANNEL(ADV_SOUND_ENGINE);

SoundEngine* SoundEngine::mInstance = NULL;

LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = NULL;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = NULL;
LPALGENEFFECTS alGenEffects = NULL;
LPALDELETEEFFECTS alDeleteEffects = NULL;
LPALEFFECTF alEffectf = NULL;
LPALEFFECTFV alEffectfv = NULL;
LPALEFFECTI alEffecti = NULL;

SoundEngine::SoundEngine() : mData(NULL), mActiveMusic(NULL), mMusicVolume(1.0f), mSpeechVolume(1.0f), mEffectEnabled(false), mFadingTime(300){
  TR_USE(ADV_SOUND_ENGINE);
#ifndef DISABLE_SOUND
  mDevice = alcOpenDevice(NULL);
  if (mDevice){
    mContext = alcCreateContext(mDevice, NULL);
    alcMakeContextCurrent(mContext);
  }
  if (!alcIsExtensionPresent(mDevice, ALC_EXT_EFX_NAME)){
    TR_ERROR("EFX exension not present");
  }
  else{
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
    alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alGenAuxiliaryEffectSlots(1, &mEffectSlot);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR){
      TR_ERROR("AL error %i", error);
    }
    alGenEffects(1, &mEffect);
    alEffecti(mEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
  }
  //init codecs
  alutInitWithoutContext(NULL, NULL);
  av_register_all();
  av_log_set_level(AV_LOG_ERROR);
#endif
}

SoundEngine::~SoundEngine(){
  for (std::map<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    delete iter->second;
  }
  mActiveSounds.clear();
  delete mActiveMusic;
#ifndef DISABLE_SOUND
  alDeleteEffects(1, &mEffect);
  alDeleteAuxiliaryEffectSlots(1, &mEffectSlot);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(mContext);
  alcCloseDevice(mDevice);
  //exit codecs
  alutExit();
#endif
}

void SoundEngine::setEAXEffect(const std::string& effect){
#ifndef DISABLE_SOUND
#ifndef DISABLE_EAX
  if (effect == "off" || effect == "none"){
    mEffectEnabled = false;
  }
  else{
    EFXREVERBPROPERTIES efxReverb;
    if (effect == "paddedcell"){
      efxReverb = REVERB_PRESET_PADDEDCELL;
    }
    else if (effect == "livingroom"){
      efxReverb = REVERB_PRESET_LIVINGROOM;
    }
    else if (effect == "cave"){
      efxReverb = REVERB_PRESET_CAVE;
    }
    else if (effect == "carpetedhallway"){
      efxReverb = REVERB_PRESET_CARPETTEDHALLWAY;
    }
    else if (effect == "alley"){
      efxReverb = REVERB_PRESET_ALLEY;
    }
    else if (effect == "mountains"){
      efxReverb = REVERB_PRESET_MOUNTAINS;
    }
    else if (effect == "parkinglot"){
      efxReverb = REVERB_PRESET_PARKINGLOT;
    }
    else if (effect == "drugged"){
      efxReverb = REVERB_PRESET_DRUGGED;
    }
    else if (effect == "concerthall"){
      efxReverb = REVERB_PRESET_CONCERTHALL;
    }
    else if (effect == "room"){
      efxReverb = REVERB_PRESET_ROOM;
    }
    else if (effect == "stoneroom"){
      efxReverb = REVERB_PRESET_STONEROOM;
    }
    else if (effect == "arena"){
      efxReverb =  REVERB_PRESET_ARENA;
    }
    else if (effect == "hallway"){
      efxReverb = REVERB_PRESET_HALLWAY;
    }
    else if (effect == "forest"){
      efxReverb = REVERB_PRESET_FOREST;
    }
    else if (effect == "quarry"){
      efxReverb = REVERB_PRESET_QUARRY;
    }
    else if (effect == "sewerpipe"){
      efxReverb = REVERB_PRESET_SEWERPIPE;
    }
    else if (effect == "dizzy"){
      efxReverb = REVERB_PRESET_DIZZY;
    }
    else if (effect == "bathroom"){
      efxReverb = REVERB_PRESET_BATHROOM;
    }
    else if (effect == "auditorium"){
      efxReverb = REVERB_PRESET_AUDITORIUM;
    }
    else if (effect == "hangar"){
      efxReverb = REVERB_PRESET_HANGAR;
    }
    else if (effect == "stonecorridor"){
      efxReverb = REVERB_PRESET_STONECORRIDOR;
    }
    else if (effect == "city"){
      efxReverb = REVERB_PRESET_CITY;
    }
    else if (effect == "plain"){
      efxReverb = REVERB_PRESET_PLAIN;
    }
    else if (effect == "underwater"){
      efxReverb = REVERB_PRESET_UNDERWATER;
    }
    else if (effect == "psychotic"){
      efxReverb = REVERB_PRESET_PSYCHOTIC;
    }
    else
      DebugBreak();
    mEffectEnabled = true;
    alEffectf(mEffect, AL_EAXREVERB_DENSITY, efxReverb.flDensity);
    alEffectf(mEffect, AL_EAXREVERB_DIFFUSION, efxReverb.flDiffusion);
    alEffectf(mEffect, AL_EAXREVERB_GAIN, efxReverb.flGain);
    alEffectf(mEffect, AL_EAXREVERB_GAINHF, efxReverb.flGainHF);
		alEffectf(mEffect, AL_EAXREVERB_GAINLF, efxReverb.flGainLF);
		alEffectf(mEffect, AL_EAXREVERB_DECAY_TIME, efxReverb.flDecayTime);
		alEffectf(mEffect, AL_EAXREVERB_DECAY_HFRATIO, efxReverb.flDecayHFRatio);
		alEffectf(mEffect, AL_EAXREVERB_DECAY_LFRATIO, efxReverb.flDecayLFRatio);
		alEffectf(mEffect, AL_EAXREVERB_REFLECTIONS_GAIN, efxReverb.flReflectionsGain);
		alEffectf(mEffect, AL_EAXREVERB_REFLECTIONS_DELAY, efxReverb.flReflectionsDelay);
		alEffectfv(mEffect, AL_EAXREVERB_REFLECTIONS_PAN, efxReverb.flReflectionsPan);
		alEffectf(mEffect, AL_EAXREVERB_LATE_REVERB_GAIN, efxReverb.flLateReverbGain);
		alEffectf(mEffect, AL_EAXREVERB_LATE_REVERB_DELAY, efxReverb.flLateReverbDelay);
		alEffectfv(mEffect, AL_EAXREVERB_LATE_REVERB_PAN, efxReverb.flLateReverbPan);
		alEffectf(mEffect, AL_EAXREVERB_ECHO_TIME, efxReverb.flEchoTime);
		alEffectf(mEffect, AL_EAXREVERB_ECHO_DEPTH, efxReverb.flEchoDepth);
		alEffectf(mEffect, AL_EAXREVERB_MODULATION_TIME, efxReverb.flModulationTime);
		alEffectf(mEffect, AL_EAXREVERB_MODULATION_DEPTH, efxReverb.flModulationDepth);
		alEffectf(mEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, efxReverb.flAirAbsorptionGainHF);
		alEffectf(mEffect, AL_EAXREVERB_HFREFERENCE, efxReverb.flHFReference);
		alEffectf(mEffect, AL_EAXREVERB_LFREFERENCE, efxReverb.flLFReference);
		alEffectf(mEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, efxReverb.flRoomRolloffFactor);
		alEffecti(mEffect, AL_EAXREVERB_DECAY_HFLIMIT, efxReverb.iDecayHFLimit);
    alAuxiliaryEffectSloti(mEffectSlot, AL_EFFECTSLOT_EFFECT, mEffect);
  }
#endif
#endif
}

SoundPlayer* SoundEngine::getSound(const std::string& name){
  SoundPlayer* plyr = mActiveSounds[name];
  if (plyr)
    return plyr;
  DataBuffer db;
  mData->getSound(name, db);
  plyr = createPlayer(name, db);
  mActiveSounds[name] = plyr;
  return plyr;
}

SoundPlayer* SoundEngine::getMusic(const std::string& name){
  SoundPlayer* plyr = NULL;
  if (name.empty()){
    return mActiveMusic;
  }
  //else
  //  plyr = mActiveSounds[name];
  if (plyr)
    return plyr;
  DataBuffer db;
  mData->getMusic(name, db);
  plyr = createPlayer(name, db);
  if (mActiveMusic){
    //crossfade
    mActiveMusic->fadeVolume(mMusicVolume, 0.0f, mFadingTime);
    mActiveSounds[mActiveMusic->getName()] = mActiveMusic;
  }
  mActiveMusic = plyr;
  if (plyr){
    plyr->fadeVolume(0.0f, mMusicVolume, mFadingTime);
  }
  return plyr;
}

VideoPlayer* SoundEngine::getMovie(const std::string& name){
  VideoPlayer* plyr = NULL;
  DataBuffer db;
  mData->getMovie(name, db);
  plyr = createVideoPlayer(name, db);
  mActiveSounds[name] = plyr;
  return plyr;
}

SoundPlayer* SoundEngine::createPlayer(const std::string& name, const DataBuffer& db){
#ifndef DISABLE_SOUND
  if (db.data == NULL)
    return NULL;
  char* tmp = tmpnam(NULL);
  std::string filename = mData->getProjectSettings()->savedir
#ifdef WIN32
+"/tmp"
#endif
+tmp+db.name;
  FILE* f = fopen(filename.c_str(), "wb");
  if (!f)
    return NULL;
  fwrite(db.data, 1, db.length, f);
  fclose(f);
  //ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  //SoundPlayer* plyr = new SimpleSoundPlayer(buffer);
  SoundPlayer* plyr = new StreamSoundPlayer(name, filename);
  return plyr;
#else
  return NULL;
#endif
}

VideoPlayer* SoundEngine::createVideoPlayer(const std::string& name, const DataBuffer& db){
#ifndef DISABLE_SOUND
  if (db.data == NULL)
    return NULL;
  char* tmp = tmpnam(NULL);
  std::string filename = mData->getProjectSettings()->savedir
#ifdef WIN32
+"/tmp"
#endif
+tmp+db.name;
  FILE* f = fopen(filename.c_str(), "wb");
  if (!f)
    return NULL;
  fwrite(db.data, 1, db.length, f);
  fclose(f);
  //ALuint buffer = alutCreateBufferFromFileImage(db.data, db.length);
  //SoundPlayer* plyr = new SimpleSoundPlayer(buffer);
  VideoPlayer* plyr = new VideoPlayer(name, filename);
  return plyr;
#else
  return NULL;
#endif
}

void SoundEngine::update(unsigned time){
  for (std::map<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && !iter->second->update(time)){
      delete iter->second;
      iter->second = NULL;
    }
  }
  if (mActiveMusic && !mActiveMusic->update(time)){
    delete mActiveMusic;
    mActiveMusic = NULL;
  }
}

void SoundEngine::removeSpeaker(CharacterObject* chr){
  for (std::map<std::string, SoundPlayer*>::iterator iter = mActiveSounds.begin(); iter != mActiveSounds.end(); ++iter){
    if (iter->second && iter->second->getSpeaker() == chr){
      delete iter->second;
      iter->second = NULL;
    }
  }
}

void SoundEngine::setMusicVolume(float volume){
  mMusicVolume = volume;
  if (mActiveMusic != NULL){
    if (!mActiveMusic->isFading())
      mActiveMusic->setVolume(volume);
  }
}

void SoundEngine::setSpeechVolume(float volume){
  mSpeechVolume = volume;
}

std::ostream& SoundEngine::save(std::ostream& out){
  out << mMusicVolume << " " << mSpeechVolume << "\n";
  out << mFadingTime << "\n";
  return out;
}

std::istream& SoundEngine::load(std::istream& in){
  in >> mMusicVolume >> mSpeechVolume;
  in >> mFadingTime;
  return in;
}


SoundPlayer::SoundPlayer(const std::string& name) : mSpeaker(NULL), mSuspensionScript(NULL), mSpokenString(NULL), mName(name), mStartVolume(1.0f), mEndVolume(1.0f), mFadeDuration(0), mCurrTime(0){
#ifndef DISABLE_SOUND
  alGenSources(1, &mSource);
  if (SoundEngine::instance()->isEffectEnabled())
    alSource3i(mSource, AL_AUXILIARY_SEND_FILTER, SoundEngine::instance()->getEffectSlot(), 0, AL_FILTER_NULL);
#endif
}

SoundPlayer::~SoundPlayer(){
  if (mSuspensionScript){
    mSuspensionScript->resume();
    mSuspensionScript->unref();
    mSuspensionScript = NULL;
  }
  if (mSpeaker){
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(mSpeaker);
    mSpeaker->setState(CharacterObject::calculateState(mSpeaker->getState(), mSpeaker->isWalking(), false));
    mSpeaker = NULL;
  }
  if (mSpokenString){
    Engine::instance()->getFontRenderer()->removeText((FontRenderer::String*)mSpokenString);
    mSpokenString = NULL;
  }
#ifndef DISABLE_SOUND
  alSource3i(mSource, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
  alDeleteSources(1, &mSource);
#endif
}

void SoundPlayer::setVolume(float volume){
#ifndef DISABLE_SOUND
  alSourcef(mSource, AL_GAIN, volume);
#endif
}

void SoundPlayer::setSuspensionScript(ExecutionContext* ctx){
  if (mSuspensionScript != NULL){
    mSuspensionScript->reset(true,true);
    mSuspensionScript->unref();
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

#ifndef DISABLE_SOUND
SimpleSoundPlayer::SimpleSoundPlayer(const std::string& name, ALuint buffer) : SoundPlayer(name), mBuffer(buffer){
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

bool SimpleSoundPlayer::update(unsigned time){
  fadeUpdate(time);
  ALint state;
  alGetSourcei(mSource, AL_SOURCE_STATE, &state);
  if (state == AL_STOPPED)
    return false;
  return true;
}

static const int BUFFER_SIZE = 19200;

StreamSoundPlayer::StreamSoundPlayer(const std::string& soundname, const std::string& filename) : 
SoundPlayer(soundname), mFilename(filename), mLooping(false), mStop(true){
  mDecodeBuffer.length = AVCODEC_MAX_AUDIO_FRAME_SIZE;
  mDecodeBuffer.data = (char*)av_malloc(mDecodeBuffer.length);
  mDecodeBuffer.used = 0;
  mDataBuffer.length = 0;
  mDataBuffer.data = NULL;
  mDataBuffer.used = 0;
  mALBuffer.length = BUFFER_SIZE;
  mALBuffer.data = new char[mALBuffer.length];
  mALBuffer.used = 0;
  openStream();
}

StreamSoundPlayer::~StreamSoundPlayer(){
  closeStream();
  //deallocate this way, as it was allocated by av_malloc
  av_free(mDecodeBuffer.data);
  mDecodeBuffer.data = NULL;
  remove(mFilename.c_str());
}

void StreamSoundPlayer::openStream(){
  alSourcei(mSource, AL_SOURCE_RELATIVE, AL_TRUE);
  alSourcei(mSource, AL_ROLLOFF_FACTOR, 0);
  alGenBuffers(3, mBuffers);
  if (av_open_input_file(&mFormat, mFilename.c_str(), NULL, 0, NULL) != 0)
    return;
  unsigned last_nb_streams;
  do{
    last_nb_streams = mFormat->nb_streams;
    av_find_stream_info(mFormat);
  } while(last_nb_streams != mFormat->nb_streams);
  for (unsigned i = 0; i < mFormat->nb_streams; ++i){
    if (mFormat->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
      mCodecContext = mFormat->streams[i]->codec;
      mCodec = avcodec_find_decoder(mCodecContext->codec_id);
      if (!mCodec)
        continue;
      if (avcodec_open(mCodecContext, mCodec) < 0)
        continue;
      mStreamNum = i;
      
      if (mCodecContext->sample_fmt == SAMPLE_FMT_U8){
        if (mCodecContext->channels == 1)
          mPCMFormat = AL_FORMAT_MONO8;
        if (mCodecContext->channels == 2)
          mPCMFormat = AL_FORMAT_STEREO8;
        if (alIsExtensionPresent("AL_EXT_MCFORMATS")){
          if (mCodecContext->channels == 4)
            mPCMFormat = alGetEnumValue("AL_FORMAT_QUAD8");
          if (mCodecContext->channels == 6)
            mPCMFormat = alGetEnumValue("AL_FORMAT_51CHN8");
        }
      }
      else if (mCodecContext->sample_fmt == SAMPLE_FMT_S16){
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
      }
      else{
        DebugBreak();
      }
      if (mPCMFormat == 0)
        continue;
    }
    /*if (mFormat->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO){
      mVidCodecContext = mFormat->streams[i]->codec;
      mVidCodec = avcodec_find_decoder(mVidCodecContext->codec_id);
      if (!mCodec)
        continue;
      if (avcodec_open(mVidCodecContext, mVidCodec) < 0){
        continue;
      }
      mVidStreamNum = i;
      if (mVidCodecContext->time_base.num>1000 && mVidCodecContext->time_base.den == 1)
        mVidCodecContext->time_base.den = 1000;

      mFrame = avcodec_alloc_frame();
      mFrameRGB = avcodec_alloc_frame();
      mVidDataBuffer.length = avpicture_get_size(PIX_FMT_RGB24, mVidCodecContext->width, mVidCodecContext->height);
      mVidDataBuffer.data = new char[mVidDataBuffer.length];
      avpicture_fill((AVPicture*)mFrameRGB, (uint8_t*)mVidDataBuffer.data, PIX_FMT_RGB24, mVidCodecContext->width, mVidCodecContext->height);
    
      mScaler = sws_getContext(mVidCodecContext->width, mVidCodecContext->height,
        mVidCodecContext->pix_fmt, mVidCodecContext->width, mVidCodecContext->height,
        PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    }*/
  }
}

void StreamSoundPlayer::closeStream(){
  alSourceStop(mSource);
  ALint queued;
  alGetSourcei(mSource, AL_BUFFERS_QUEUED, &queued);
  if (queued > 0){
    ALuint curBuf;
    alSourceUnqueueBuffers(mSource, 1, &curBuf);
  }
  alSourcei(mSource, AL_BUFFER, AL_NONE);
  alDeleteBuffers(3, mBuffers);
  if (mFormat)
    av_close_input_file(mFormat);
}

#ifdef WIN32
#define VHALIGNCALL16(x) x
/*\
{\
  _asm { mov ebx, esp }\
  _asm { and esp, 0xfffffff0 }\
  _asm { sub esp, 12 }\
  _asm { push ebx }\
  x;\
  _asm { pop ebx }\
  _asm { mov esp, ebx }\
}*/
#else
#define VHALIGNCALL16(x) x
#endif

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
      AVPacket tmppkt;
      av_init_packet(&tmppkt);
      tmppkt.data = (uint8_t*)mDataBuffer.data;
      tmppkt.size = insize;
      VHALIGNCALL16(length = avcodec_decode_audio3(mCodecContext, (int16_t*)mDecodeBuffer.data, &size, &tmppkt));//(uint8_t*)mDataBuffer.data, insize));
      while(length == 0){
        if (size > 0)
          break;
        getNextPacket();
        if (insize == mDataBuffer.used)
          break;
        insize = mDataBuffer.used;
        memset(mDataBuffer.data+insize, 0, FF_INPUT_BUFFER_PADDING_SIZE);
        VHALIGNCALL16(length = avcodec_decode_audio3(mCodecContext, (int16_t*)mDecodeBuffer.data, &size, &tmppkt));//(uint8_t*)mDataBuffer.data, insize));
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
  int read;
  do{
    read = av_read_frame(mFormat, &packet);
    if (read < 0){
      if (!mLooping)
        return;
      //start all over again
      av_seek_frame(mFormat, 0, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
      read = av_read_frame(mFormat, &packet);
      if (read < 0){
        return;
      }
    }
    if (packet.stream_index == mStreamNum){
      unsigned idx = mDataBuffer.used;
      if (idx+packet.size+FF_INPUT_BUFFER_PADDING_SIZE > mDataBuffer.length){
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
    /*else if (packet.stream_index == mVidStreamNum){
      int frame_finished;
      avcodec_decode_video(mVidCodecContext, mFrame, &frame_finished, packet.data, packet.size);
      if (frame_finished){
        sws_scale(mScaler, mFrame->data, mFrame->linesize, 0, mVidCodecContext->height,
          mFrameRGB->data, mFrameRGB->linesize);
        av_free_packet(&packet);
        break;
      }
      av_free_packet(&packet);
    }*/
  } while(read >= 0);
}

void StreamSoundPlayer::play(bool looping){
  mLooping = looping;
  mStop = false;
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
  mStop = true;
  mLooping = false;
  alSourceStop(mSource);
  //VHALIGNCALL16(av_seek_frame(mFormat, 0, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY));
}

bool StreamSoundPlayer::update(unsigned time){
  fadeUpdate(time);
  ALint processed;
  ALint queued;
  alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
  alGetSourcei(mSource, AL_BUFFERS_QUEUED, &queued);
  if (processed == queued){
    //all buffers empty
    //ALint state;
    //alGetSourcei(mSource, AL_SOURCE_STATE, &state);
    unsigned bytes = decode();
    if (bytes > 0){
      ALuint curBuf;
      alSourceUnqueueBuffers(mSource, 1, &curBuf);
      if (curBuf != 0){
        alBufferData(curBuf, mPCMFormat, mALBuffer.data, bytes, mCodecContext->sample_rate);
        alSourceQueueBuffers(mSource, 1, &curBuf);
      }
    }
    else
      mStop = true;
    if (mStop){
      if (mLooping){
        //stop was not requested, so restart
        closeStream();
        openStream();
        play(mLooping);
        return true;
      }
      return false;
    }
    return true;
  }
  for (ALint i = 0; i < processed; ++i){
    if (mStop)
      break;
    unsigned bytes = decode();
    if (bytes > 0){
      ALuint curBuf;
      alSourceUnqueueBuffers(mSource, 1, &curBuf);
      if (curBuf != 0){
        alBufferData(curBuf, mPCMFormat, mALBuffer.data, bytes, mCodecContext->sample_rate);
        alSourceQueueBuffers(mSource, 1, &curBuf);
      }
    }
    else{
      mStop = true;
    }
  }
  ALint state;
  alGetSourcei(mSource, AL_SOURCE_STATE, &state);
  if (state != AL_PLAYING)
    alSourcePlay(mSource);
  return true;
}

VideoPlayer::VideoPlayer(const std::string& soundname, const std::string& filename) : 
StreamSoundPlayer(soundname, filename){
}

VideoPlayer::~VideoPlayer(){
}

#endif
