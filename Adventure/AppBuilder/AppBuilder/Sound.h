#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <map>
#ifndef DISABLE_SOUND
#ifdef WIN32
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#endif
#include "AdvDoc.h"
#include "ExecutionContext.h"

namespace adv{

class SoundPlayer;
class VideoPlayer;
class AdvDocument;
class CharacterObject;
class ExecutionContext;

class SoundEngine{
public:
  enum PlayerFlags{
    PLAYER_DEFAULT=0,
    PLAYER_CREATE_ALWAYS=1,
    PLAYER_UNMANAGED=2,
  };
  ~SoundEngine();
  static void init() {mInstance = new SoundEngine();}
  static void deinit() {delete mInstance;}
  static SoundEngine* instance() {return mInstance;}
  void setData(AdvDocument* doc){mData = doc;}
  SoundPlayer* getSound(const std::string& name, int flags);
  SoundPlayer* getSound(const std::string& name, bool effectEnabled, int flags);
  SoundPlayer* getMusic(const std::string& name);
  SoundPlayer* getMusic(const std::string& name, bool effectEnabled);
  VideoPlayer* getMovie(const std::string& name, bool isSwf);
  void update(unsigned time);
  void removeSpeaker(CharacterObject* chr);
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);
  void setMusicVolume(float volume);
  float getMusicVolume() {return mMusicVolume;}
  void setSpeechVolume(float volume);
  float getSpeechVolume() {return mSpeechVolume;}
  void setEAXEffect(const std::string& effect);
  void setFadingTime(unsigned time) {mFadingTime = time;}
  unsigned getFadingTime() {return mFadingTime;}
#ifndef DISABLE_SOUND
  ALuint getEffectSlot() {return mEffectSlot;}
#endif
  bool isEffectEnabled() {return mCurrentEffect != "none";}
  void removeSoundPlayer(SoundPlayer* plyr);
  void setSpeedFactor(float speed);
protected:
  SoundEngine();
  SoundPlayer* createPlayer(const std::string& name, const DataBuffer& db, bool effectEnabled);
  VideoPlayer* createVideoPlayer(const std::string& name, const DataBuffer& db, bool isSwf);
  static SoundEngine* mInstance;
  AdvDocument* mData;
#ifndef DISABLE_SOUND
  ALCdevice* mDevice;
  ALCcontext* mContext;
  ALuint mEffectSlot;
  ALuint mEffect;
#endif
  std::multimap<std::string, SoundPlayer*> mActiveSounds;
  SoundPlayer* mActiveMusic;
  VideoPlayer* mActiveVideo;
  float mMusicVolume;
  float mSpeechVolume;
  std::string mCurrentEffect;
  unsigned mFadingTime;
  float mSpeedFactor;
};

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
  virtual void setAutoDelete(bool del) {}
  virtual bool hasAutoDeletion() {return true;}
  void setSpeed(float factor);
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
};

class VideoPlayer{
public:
  virtual ~VideoPlayer() {}
  virtual void play(bool looping)=0;
  virtual void stop()=0;
  virtual bool update(unsigned time)=0;
  virtual void initLayer(int x, int y, int width, int height)=0;
  virtual void setSuspensionScript(ExecutionContext* ctx)=0;
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

namespace adv{

class StreamSoundPlayer : public SoundPlayer{
public:
  StreamSoundPlayer(const std::string& soundname, bool effectEnabled);
  virtual ~StreamSoundPlayer();
  bool openStream(const std::string& filename);
  bool openStream(const DataBuffer& buffer);
  void play(bool looping);
  void stop();
  bool update(unsigned time);
  virtual void seek(int time);
  virtual bool isLooping() {return mLooping;}
  virtual void setAutoDelete(bool del) {mAutoDelete = del;}
  bool hasAutoDeletion() {return mAutoDelete;}
protected:
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
};

class BlitObject;

class StreamVideoPlayer : public VideoPlayer, public StreamSoundPlayer{
public:
  StreamVideoPlayer(const std::string& videoname);
  virtual ~StreamVideoPlayer();
  void play(bool looping) {StreamSoundPlayer::play(looping);}
  void stop() {StreamSoundPlayer::stop();}
  void setSuspensionScript(ExecutionContext* ctx) {StreamSoundPlayer::setSuspensionScript(ctx);}
  bool update(unsigned time);
  void initLayer(int x, int y, int width, int height);
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
  DataBuffer mVidDataBuffer;
  int mClock;

  Vec2i mRenderPos;
  Vec2f mScale;
  BlitObject* mLayer;
};

}

#endif

#endif
