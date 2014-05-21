#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <string>
#include <list>

#ifndef DISABLE_SOUND
#ifdef WIN32
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#endif

#include <system/thread.h>

#include "AdvDoc.h"

namespace adv{

class CharacterObject;
class ExecutionContext;

class SoundPlayer{
public:
  SoundPlayer(const std::string& name, bool effectEnabled);
  virtual ~SoundPlayer();
  virtual void play(bool looping)=0;
  virtual void stop()=0;
  virtual void seek(int time)=0;
  void setVolume(float volume);
  virtual bool update(unsigned time)=0;
  void setSpeaker(CharacterObject* chr) {mSpeaker = chr;}
  CharacterObject* getSpeaker() {return mSpeaker;}
  void setSuspensionScript(ExecutionContext* ctx);
  void setSpokenString(/*FontRenderer::String*/void* string) {mSpokenString = string;}
  void fadeVolume(float from, float to, unsigned time);
  bool isFading() {return mFadeDuration != 0;}
  std::string getName() {return mName;}
  bool hasEffect() {return mEffectEnabled;}
  virtual bool isLooping() {return false;}
  virtual bool isPlaying() {return false;}
  virtual void setAutoDelete(bool del) {}
  virtual bool hasAutoDeletion() {return true;}
  void setSpeed(float factor);
  virtual void realize();
protected:
  bool fadeUpdate(unsigned time);
#ifndef DISABLE_SOUND
  ALuint mSource;
#endif
  CharacterObject* mSpeaker;
  ExecutionContext* mSuspensionScript;
  /*FontRenderer::String*/void* mSpokenString;
  std::string mName;
  float mStartVolume;
  float mEndVolume;
  unsigned mFadeDuration;
  unsigned mCurrTime;
  bool mEffectEnabled;
  float mSpeed;
};

class VideoPlayer{
public:
  virtual ~VideoPlayer() {}
  virtual void play(bool looping)=0;
  virtual void stop()=0;
  virtual bool update(unsigned time)=0;
  virtual void render(unsigned time)=0;
  virtual void initLayer(int x, int y, int width, int height)=0;
  virtual void setSuspensionScript(ExecutionContext* ctx)=0;
  virtual void realize()=0;
};

}


#ifndef DISABLE_SOUND

namespace adv{

class SimpleSoundPlayer : public SoundPlayer{
public:
  SimpleSoundPlayer(const std::string& name, ALuint buffer, bool effectEnabled);
  virtual ~SimpleSoundPlayer();
  void play(bool looping);
  void stop();
  bool update(unsigned time);
  void seek(int time) {}
  virtual void realize();
protected:
#ifndef DISABLE_SOUND
  ALuint mBuffer;
#endif
};

}

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct SwrContext;

namespace adv{

class StreamSoundPlayer : public SoundPlayer{
public:
  StreamSoundPlayer(const std::string& soundname, bool effectEnabled);
  virtual ~StreamSoundPlayer();
  void setStream(const std::string& filename);
  void setStream(const DataBuffer& buffer);
  void play(bool looping);
  void stop();
  bool update(unsigned time);
  virtual void seek(int time);
  virtual bool isLooping() {return mLooping;}
  virtual void setAutoDelete(bool del) {mAutoDelete = del;}
  bool hasAutoDeletion() {return mAutoDelete;}
  virtual void realize();
  bool isPlaying() {return mPlay;}
protected:
  bool openStream(const std::string& filename);
  bool openStream(const DataBuffer& buffer);
  unsigned decode();
  bool getNextPacket();
  void closeStream();
  bool openStreamInternal();
  virtual bool openStreamHook(int currStream) {return false;}
  virtual bool getPacketHook(AVPacket& packet) {return false;}
  ALuint mBuffers[3];
  ALenum mPCMFormat;
  std::string mFilename;
  AVFormatContext* mFormat;
  AVCodecContext* mCodecContext;
  AVCodec* mCodec;
  SwrContext* mResampler;
  int mStreamNum;
  DataBuffer mDataBuffer;
  DataBuffer mDecodeBuffer;
  DataBuffer mALBuffer;
  bool mLooping;
  bool mStop;
  bool mPlay;
  bool mAutoDelete;
  unsigned char* mMemoryBuffer;
  void* mMemoryStream;

  const DataBuffer* mStreamDB;
  std::string mStreamF;
};

class BlitObject;

class StreamVideoPlayer : public VideoPlayer, public StreamSoundPlayer{
public:
  StreamVideoPlayer(const std::string& videoname);
  virtual ~StreamVideoPlayer();
  void play(bool looping) {StreamSoundPlayer::play(looping);}
  void stop() {StreamSoundPlayer::stop();}
  void setSuspensionScript(ExecutionContext* ctx) {StreamSoundPlayer::setSuspensionScript(ctx);}
  virtual bool update(unsigned time);
  virtual void render(unsigned time);
  void initLayer(int x, int y, int width, int height);
  virtual void realize() {StreamSoundPlayer::realize();}
protected:
  virtual bool openStreamHook(int currStream);
  virtual bool getPacketHook(AVPacket& packet);
  /*unsigned decode();
  void getNextPacket();
  void openStream();
  void closeStream();
  ALuint mBuffers[3];
  ALenum mPCMFormat;
  std::string mFilename;
  AVFormatContext* mFormat;
  AVCodecContext* mCodecContext;
  AVCodec* mCodec;
  int mStreamNum;
  DataBuffer mDataBuffer;
  DataBuffer mDecodeBuffer;
  DataBuffer mALBuffer;
  bool mLooping;
  bool mStop;*/

  AVCodecContext* mVidCodecContext;
  AVCodec* mVidCodec;
  AVFrame* mFrame;
  std::list<AVFrame*> mFramesRGB;
  SwsContext* mScaler;
  int mVidStreamNum;
  unsigned mVidDataBufferLength;
  int mClock;

  Vec2i mRenderPos;
  Vec2f mScale;
#define NUM_LAYERS 3
  BlitObject* mLayer[NUM_LAYERS];
  int mCurrLayer;
  bool mFirstFrame;
  CGE::Mutex mMutex;
};

}

#endif

#endif
