#include "SoundPlayer.h"

#include "Sound.h"
#include "Engine.h"

#ifdef UNIX
#define UINT64_C(val) (uint64_t)val
#endif
#ifndef UINT64_C
#define UINT64_C(val) val##ui64
#endif
#ifdef UNIX
#define INT64_C(val) (int64_t)val
#endif
#ifndef INT64_C
#define INT64_C(val) val##i64
#endif

#ifdef WIN32
#pragma warning( disable : 4244 )
#endif

#ifndef DISABLE_SOUND
#include <AL/alut.h>
#ifdef UNIX
#include <AL/efx.h>
#else
#include <efx.h>
#include <efx-creative.h>
#endif
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};
#endif

#include <system/allocation.h>

using namespace adv;

SoundPlayer::SoundPlayer(const std::string& name, bool effectEnabled) : mSpeaker(NULL), mSuspensionScript(NULL), mSpokenString(NULL), 
mName(name), mStartVolume(1.0f), mEndVolume(1.0f), mFadeDuration(0), mCurrTime(0), mEffectEnabled(effectEnabled), mSpeed(0){
#ifndef DISABLE_SOUND
  mSource = 0;
#endif
}

void SoundPlayer::realize(){
#ifndef DISABLE_SOUND
  alGenSources(1, &mSource);
  if (mEffectEnabled)
    alSource3i(mSource, AL_AUXILIARY_SEND_FILTER, SoundEngine::instance()->getEffectSlot(), 0, AL_FILTER_NULL);
#endif
  if (mSpeed != 0)
    setSpeed(mSpeed);
}

SoundPlayer::~SoundPlayer(){
  if (mSuspensionScript){
    mSuspensionScript->resume();
    mSuspensionScript->unref();
    mSuspensionScript = NULL;
  }
  if (mSpeaker){
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(mSpeaker, false);
    mSpeaker->setTalking(false);
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

void SoundPlayer::setSpeed(float factor){
  mSpeed = factor;
#ifndef DISABLE_SOUND
  if (mSource != 0)
    alSourcef(mSource, AL_PITCH, factor);
#endif
}

#ifndef DISABLE_SOUND
SimpleSoundPlayer::SimpleSoundPlayer(const std::string& name, ALuint buffer, bool effectEnabled) : SoundPlayer(name, effectEnabled), mBuffer(buffer){
}

void SimpleSoundPlayer::realize(){
  SoundPlayer::realize();
  alSourcei(mSource, AL_BUFFER, mBuffer);
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

//TR_CHANNEL_LVL(ADV_Sound_Player, TRACE_DEBUG_DETAIL);
TR_CHANNEL(ADV_Sound_Player);

static const int BUFFER_SIZE = 19200;

StreamSoundPlayer::StreamSoundPlayer(const std::string& soundname, bool effectEnabled) : 
SoundPlayer(soundname, effectEnabled), mCodecContext(NULL), mCodec(NULL), mStreamNum(-1), 
mLooping(false), mStop(true), mPlay(false), mAutoDelete(true) {
  TR_USE(ADV_Sound_Player);
  TR_DEBUG("Creating sound player %s", soundname.c_str());
  mDecodeBuffer.length = AVCODEC_MAX_AUDIO_FRAME_SIZE;
  mDecodeBuffer.data = (char*)av_malloc(mDecodeBuffer.length);
  mDecodeBuffer.used = 0;
  mDataBuffer.length = 0;
  mDataBuffer.data = NULL;
  mDataBuffer.used = 0;
  mALBuffer.length = BUFFER_SIZE;
  mALBuffer.data = new char[mALBuffer.length];
  mALBuffer.used = 0;
}

void StreamSoundPlayer::realize(){
  SoundPlayer::realize();
  if (mStreamDB != NULL){
    if (!openStream(*mStreamDB)){
      delete mStreamDB;
      mStreamDB = NULL;
    }
  }
  else
    openStream(mStreamF);
}

StreamSoundPlayer::~StreamSoundPlayer(){
  closeStream();
  //deallocate this way, as it was allocated by av_malloc
  av_free(mDecodeBuffer.data);
  mDecodeBuffer.data = NULL;
  //remove(mFilename.c_str());
}

void StreamSoundPlayer::setStream(const std::string& filename){
  mStreamF = filename;
  mStreamDB = NULL;
}

void StreamSoundPlayer::setStream(const DataBuffer& buffer){
  mStreamF = "";
  mStreamDB = &buffer;
}

bool StreamSoundPlayer::openStream(const std::string& filename){
  mFilename = filename;
  mMemoryBuffer = NULL;
  mMemoryStream = NULL;
  alSourcei(mSource, AL_SOURCE_RELATIVE, AL_TRUE);
  alSourcei(mSource, AL_ROLLOFF_FACTOR, 0);
  alGenBuffers(3, mBuffers);

  if (avformat_open_input(&mFormat, mFilename.c_str(), NULL, NULL) != 0)
    return false;
  return openStreamInternal();
}

static int readMemStream(void* opaque, uint8_t* buf, int buf_size){
  TR_USE(ADV_Sound_Player);
  TR_DETAIL("read %i bytes", buf_size);
  DataBuffer* db = (DataBuffer*)opaque;
  int dbsize = db->length-db->used;
  int size = buf_size < dbsize ? buf_size : dbsize;
  memcpy(buf, db->data+db->used, size);
  db->used += size;
  return size;
}

static int64_t seekMemStream(void* opaque, int64_t offset, int whence){
  TR_USE(ADV_Sound_Player);
  TR_DETAIL("seek offset %i, whence %i", offset, whence);
  DataBuffer* db = (DataBuffer*)opaque;
  if (whence == AVSEEK_SIZE){
    return db->length;
  }
#ifndef FFMPEG_OLD_API
  whence &= ~AVSEEK_FORCE;
#endif
  if (whence == SEEK_SET){
    db->used = offset;
    return offset;
  }
  else{
    TR_BREAK("whence value (%i) unepected", whence);
  }
  return 0;
}

bool StreamSoundPlayer::openStream(const DataBuffer& buffer){
  TR_USE(ADV_Sound_Player);
  mFilename = buffer.name;
  alSourcei(mSource, AL_SOURCE_RELATIVE, AL_TRUE);
  alSourcei(mSource, AL_ROLLOFF_FACTOR, 0);
  alGenBuffers(3, mBuffers);

  mMemoryBuffer = (unsigned char*)av_malloc(BUFFER_SIZE+FF_INPUT_BUFFER_PADDING_SIZE);
  AVIOContext* ctx = avio_alloc_context(mMemoryBuffer, BUFFER_SIZE, 0, (void*)&buffer, readMemStream, NULL, seekMemStream);
  AVProbeData probe;
  probe.filename = mFilename.c_str();
  probe.buf_size = buffer.length-AVPROBE_PADDING_SIZE;
  probe.buf = (unsigned char*)buffer.data;
  AVInputFormat* fmt = av_probe_input_format(&probe, 1);
  if (fmt){
    TR_DEBUG("format %s", fmt->long_name);
  }
  mFormat = avformat_alloc_context();
  mFormat->pb = ctx;
  if (avformat_open_input(&mFormat, mFilename.c_str(), fmt, NULL) != 0){
    TR_WARN("open stream failed for %s", mFilename.c_str());
    av_free(ctx);
    av_free(mMemoryBuffer);
    //delete &buffer; //is deleted outside
    mFormat = NULL;
    mMemoryStream = NULL;
    return false;
  }
  TR_DEBUG("stream opened successfully");
  mMemoryStream = ctx;
  return openStreamInternal();
}

bool StreamSoundPlayer::openStreamInternal(){
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
      
      if (mCodecContext->sample_fmt == AV_SAMPLE_FMT_U8){
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
      else if (mCodecContext->sample_fmt == AV_SAMPLE_FMT_S16){
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
        TR_USE(ADV_Sound_Player);
        TR_BREAK("Sample format %i unexpected", mCodecContext->sample_fmt);
      }
      if (mPCMFormat == 0)
        continue;
      mFormat->streams[i]->discard = AVDISCARD_DEFAULT;
      continue;
    }
    if (openStreamHook(i)){
      mFormat->streams[i]->discard = AVDISCARD_DEFAULT;
    }
    else
      mFormat->streams[i]->discard = AVDISCARD_ALL;
  }
  return true;
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
  if (mFormat){
    if (mMemoryStream != NULL){
      avformat_close_input(&mFormat);
      AVIOContext* ctx = (AVIOContext*)mMemoryStream;
      DataBuffer* db = (DataBuffer*)ctx->opaque;
      av_free(ctx);
      av_free(mMemoryBuffer);
      delete db;
      mMemoryStream = NULL;
    }
    else
      av_close_input_file(mFormat);
  }
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

bool StreamSoundPlayer::getNextPacket(){
  AVPacket packet;
  int read;
  do{
    read = av_read_frame(mFormat, &packet);
    if (read < 0){
      if (!mLooping)
        return false;
      //start all over again
      av_seek_frame(mFormat, 0, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
      read = av_read_frame(mFormat, &packet);
      if (read < 0){
        return false;
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
    else{
      bool isFinished = getPacketHook(packet);
      if (isFinished && mCodecContext == NULL)
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
  return true;
}

void StreamSoundPlayer::play(bool looping){
  mLooping = looping;
  mStop = false;
  mPlay = true;
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

void StreamSoundPlayer::seek(int time){
  av_seek_frame(mFormat, 0, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
  for (int i = 0; i < 3; ++i){
    alSourceUnqueueBuffers(mSource, 1, &mBuffers[i]);
  }
}

bool StreamSoundPlayer::update(unsigned time){
  TR_USE(ADV_Sound_Player);
  TR_DETAIL("updating with time %i", time);
  if (!mPlay)
    return true;
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
        bool inmemory = mMemoryStream != NULL;
        DataBuffer* db = NULL;
        if (inmemory){
          AVIOContext* ctx = (AVIOContext*)mMemoryStream;
          db = (DataBuffer*)ctx->opaque;
          db->used = 0;
          ctx->opaque = NULL;
        }
        closeStream();
        if (inmemory)
          openStream(*db);
        else
          openStream(mFilename);
        play(mLooping);
        return true;
      }
      mPlay = false;
      return !mAutoDelete;
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

TR_CHANNEL(ADV_Video);

StreamVideoPlayer::StreamVideoPlayer(const std::string& soundname) : 
StreamSoundPlayer(soundname, false), mClock(0)/*, mLayer(NULL)*/, mFirstFrame(true){
  for (int i = 0; i < NUM_LAYERS; ++i)
    mLayer[i] = NULL;
}

StreamVideoPlayer::~StreamVideoPlayer(){
  av_free(mFrame);
  for (std::list<AVFrame*>::iterator iter = mFramesRGB.begin(); iter != mFramesRGB.end(); ++iter){
    delete [] (*iter)->data[0];
    av_free(*iter);
  }
  avcodec_close(mVidCodecContext);
  for (int i = 0; i < NUM_LAYERS; ++i)
    delete mLayer[i];
}

bool StreamVideoPlayer::openStreamHook(int i){
  if (mFormat->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
    mVidCodecContext = mFormat->streams[i]->codec;
    mVidCodec = avcodec_find_decoder(mVidCodecContext->codec_id);
    if (!mVidCodec)
      return false;
    if (avcodec_open(mVidCodecContext, mVidCodec) < 0){
      return false;
    }
    mVidStreamNum = i;
    if (mVidCodecContext->time_base.num>1000 && mVidCodecContext->time_base.den == 1)
      mVidCodecContext->time_base.den = 1000;

    mFrame = avcodec_alloc_frame();
    mVidDataBufferLength = avpicture_get_size(PIX_FMT_RGB24, mVidCodecContext->width, mVidCodecContext->height);

    mScaler = sws_getContext(mVidCodecContext->width, mVidCodecContext->height,
      mVidCodecContext->pix_fmt, mVidCodecContext->width, mVidCodecContext->height,
      PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    return true;
  }
  return false;
}

/*static void saveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

    // Close file
    fclose(pFile);
}*/


bool StreamVideoPlayer::getPacketHook(AVPacket& packet){
  if (packet.stream_index == mVidStreamNum){
    int frame_finished;
    avcodec_decode_video2(mVidCodecContext, mFrame, &frame_finished, &packet);
    if (frame_finished){
      AVFrame* frameRGB = avcodec_alloc_frame();
      uint8_t* data = new uint8_t[mVidDataBufferLength];
      avpicture_fill((AVPicture*)frameRGB, data, PIX_FMT_RGB24, mVidCodecContext->width, mVidCodecContext->height);
      sws_scale(mScaler, mFrame->data, mFrame->linesize, 0, mVidCodecContext->height,
        frameRGB->data, frameRGB->linesize);
      if (mFrame->pts == AV_NOPTS_VALUE)
        frameRGB->pts = mFrame->pkt_pts;
        //frameRGB->pts = mFrame->best_effort_timestamp;
      else
        frameRGB->pts = mFrame->pts;
      av_free_packet(&packet);
      mMutex.lock();
      mFramesRGB.push_back(frameRGB);
      mMutex.unlock();
      /*static int count = 0;
      if (count < 10){
        saveFrame(mFrameRGB, mVidCodecContext->width, mVidCodecContext->height, count);
        ++count;
      }*/
      return true;
    }
    av_free_packet(&packet);
  }
  return false;
}

void StreamVideoPlayer::initLayer(int x, int y, int width, int height){
  mRenderPos.x = x;
  mRenderPos.y = y;
  mScale.x = width/(float)mVidCodecContext->width;
  mScale.y = height/(float)mVidCodecContext->height;
  for (int i = 0; i < NUM_LAYERS; ++i){
    mLayer[i] = new BlitObject(mVidCodecContext->width, mVidCodecContext->height, DEPTH_VIDEO_LAYER);
    mLayer[i]->realizeEmpty(GL_RGB);
  }
  mCurrLayer = 0;
}

bool StreamVideoPlayer::update(unsigned time){
  bool cont = false;
  if (!mFirstFrame)
    mClock += time;
  if (mCodecContext != NULL){ //we have an audio stream
      cont = StreamSoundPlayer::update(time);
  }
  else{
    if (mStop)
      return false;
    if (mFramesRGB.empty()){
      cont = getNextPacket();
    }
    if (!mFramesRGB.empty()){
      cont = true;
    }
  }
  if (!cont)
    mFirstFrame = true;
  return cont;
}

void StreamVideoPlayer::render(unsigned time){
  static int frameNum = 0;
  static int frameTime = 0;
  static int ptsTime = 0;
  frameTime += time;
  TR_USE(ADV_Video);
  mMutex.lock();
  //TR_DETAIL("Time is %i clock %i, there are %i frames queued", time, mClock, mFramesRGB.size());
  if (!mFramesRGB.empty()){
    mFirstFrame = false;
    AVFrame* frameRGB = mFramesRGB.front();
    bool firstLoop = true;
    //while because of frame skip to catch up
    while (mClock >= mVidCodecContext->time_base.num*1000/(float)mVidCodecContext->time_base.den*frameRGB->pts){
      int ptsNew = mVidCodecContext->time_base.num*1000/(float)mVidCodecContext->time_base.den*frameRGB->pts;
      if (firstLoop){
        //make sure minimum display time
        if (frameTime < 20/*ptsNew-ptsTime*/)
          break;
        //TR_INFO("Frame %i was displayed %i ms, queue %i", frameNum, frameTime, mFramesRGB.size());
        mCurrLayer = (mCurrLayer+1)%NUM_LAYERS;
        firstLoop = false;
      }
      //get video packet
      mFramesRGB.pop_front();
      ptsTime = ptsNew;
      ++frameNum;
      frameTime = 0;
      mLayer[mCurrLayer]->updateTexture(mVidCodecContext->width, mVidCodecContext->height, frameRGB->data[0]);
      delete [] frameRGB->data[0];
      av_free(frameRGB);
      if (mFramesRGB.empty()){
        TR_WARN("Video queue ran empty");
        break;
      }
      frameRGB = mFramesRGB.front();
    }
  }
  else{
    TR_DEBUG("Video queue is empty");
  }
  mMutex.unlock();
  if (mLayer && !mFirstFrame){
    //TR_INFO("Rendering frame %i buffer %i", frameNum, mCurrLayer);
    Engine::instance()->beginRendering();
    mLayer[mCurrLayer]->render(mRenderPos, mScale, Vec2i());
    Engine::instance()->endRendering();
  }
}

#endif
