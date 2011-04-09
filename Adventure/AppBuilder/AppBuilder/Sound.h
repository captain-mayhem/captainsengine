#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <map>
#ifndef DISABLE_SOUND
#include <al.h>
#include <alc.h>
#endif
#include "AdvDoc.h"

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
  void update();
  void removeSpeaker(CharacterObject* chr);
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);
  void setMusicVolume(int volume);
protected:
  SoundEngine();
  SoundPlayer* createPlayer(const std::string& name, const DataBuffer& db);
  static SoundEngine* mInstance;
  AdvDocument* mData;
#ifndef DISABLE_SOUND
  ALCdevice* mDevice;
  ALCcontext* mContext;
#endif
  std::map<std::string, SoundPlayer*> mActiveSounds;
  SoundPlayer* mActiveMusic;
  int mMusicVolume;
};

class SoundPlayer{
public:
  SoundPlayer(const std::string& name);
  virtual ~SoundPlayer();
  virtual void play(bool looping)=0;
  virtual void stop()=0;
  void setVolume(float volume);
  virtual bool update()=0;
  void setSpeaker(CharacterObject* chr) {mSpeaker = chr;}
  CharacterObject* getSpeaker() {return mSpeaker;}
  void setSuspensionScript(ExecutionContext* ctx) {mSuspensionScript = ctx;}
  void setSpokenString(/*FontRenderer::String*/void* string) {mSpokenString = string;}
protected:
#ifndef DISABLE_SOUND
  ALuint mSource;
#endif
  CharacterObject* mSpeaker;
  ExecutionContext* mSuspensionScript;
  /*FontRenderer::String*/void* mSpokenString;
  std::string mName;
};

#ifndef DISABLE_SOUND

class SimpleSoundPlayer : public SoundPlayer{
public:
  SimpleSoundPlayer(const std::string& name, ALuint buffer);
  virtual ~SimpleSoundPlayer();
  void play(bool looping);
  void stop();
  bool update();
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
  bool update();
protected:
  unsigned decode();
  void getNextPacket();
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
