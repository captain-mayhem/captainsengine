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
class VideoPlayer;
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
  VideoPlayer* getMovie(const std::string& name, int x, int y, int width, int height);
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
  VideoPlayer* createVideoPlayer(const std::string& name, const DataBuffer& db, int x, int y, int width, int height);
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
struct AVPacket;
struct SwsContext;

class StreamSoundPlayer : public SoundPlayer{
public:
  StreamSoundPlayer(const std::string& soundname);
  virtual ~StreamSoundPlayer();
  bool openStream(const std::string& filename);
  void play(bool looping);
  void stop();
  bool update(unsigned time);
protected:
  unsigned decode();
  bool getNextPacket();
  void closeStream();
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

  /*AVCodecContext* mVidCodecContext;
  AVCodec* mVidCodec;
  AVFrame* mFrame;
  AVFrame* mFrameRGB;
  SwsContext* mScaler;
  int mVidStreamNum;
  DataBuffer mVidDataBuffer;*/
};

class BlitObject;

class VideoPlayer : public StreamSoundPlayer{
public:
  VideoPlayer(const std::string& videoname);
  virtual ~VideoPlayer();
  //void play(bool looping);
  //void stop();
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

#endif

#endif
