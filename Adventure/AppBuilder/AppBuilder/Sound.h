#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <map>
#ifndef DISABLE_SOUND
#include <al.h>
#include <alc.h>
#endif
#include "AdvDoc.h"
#include "ExecutionContext.h"

class SoundPlayer;
class AdvDocument;
class CharacterObject;
class ExecutionContext;

class SoundEngine{
public:
  ~SoundEngine();
  static void init() {mInstance = new SoundEngine();}
  static void deinit() {delete mInstance;}
  static SoundEngine* instance() {return mInstance;}
  void setData(AdvDocument* doc){mData = doc;}
  SoundPlayer* getSound(const std::string& name);
  SoundPlayer* getMusic(const std::string& name);
  SoundPlayer* getMovie(const std::string& name);
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
  bool isEffectEnabled() {return mEffectEnabled;}
protected:
  SoundEngine();
  SoundPlayer* createPlayer(const std::string& name, const DataBuffer& db);
  static SoundEngine* mInstance;
  AdvDocument* mData;
#ifndef DISABLE_SOUND
  ALCdevice* mDevice;
  ALCcontext* mContext;
  ALuint mEffectSlot;
  ALuint mEffect;
#endif
  std::map<std::string, SoundPlayer*> mActiveSounds;
  SoundPlayer* mActiveMusic;
  float mMusicVolume;
  float mSpeechVolume;
  bool mEffectEnabled;
  unsigned mFadingTime;
};

class SoundPlayer{
public:
  SoundPlayer(const std::string& name);
  virtual ~SoundPlayer();
  virtual void play(bool looping)=0;
  virtual void stop()=0;
  void setVolume(float volume);
  virtual bool update(unsigned time)=0;
  void setSpeaker(CharacterObject* chr) {mSpeaker = chr;}
  CharacterObject* getSpeaker() {return mSpeaker;}
  void setSuspensionScript(ExecutionContext* ctx);
  void setSpokenString(/*FontRenderer::String*/void* string) {mSpokenString = string;}
  void fadeVolume(float from, float to, unsigned time);
  bool isFading() {return mFadeDuration != 0;}
  std::string getName() {return mName;}
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
};

#ifndef DISABLE_SOUND

class SimpleSoundPlayer : public SoundPlayer{
public:
  SimpleSoundPlayer(const std::string& name, ALuint buffer);
  virtual ~SimpleSoundPlayer();
  void play(bool looping);
  void stop();
  bool update(unsigned time);
protected:
#ifndef DISABLE_SOUND
  ALuint mBuffer;
#endif
};

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwsContext;

class StreamSoundPlayer : public SoundPlayer{
public:
  StreamSoundPlayer(const std::string& soundname, const std::string& filename);
  virtual ~StreamSoundPlayer();
  void play(bool looping);
  void stop();
  bool update(unsigned time);
protected:
  unsigned decode();
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
  bool mStop;

  AVCodecContext* mVidCodecContext;
  AVCodec* mVidCodec;
  AVFrame* mFrame;
  AVFrame* mFrameRGB;
  SwsContext* mScaler;
  int mVidStreamNum;
  DataBuffer mVidDataBuffer;
};

#endif

#endif
