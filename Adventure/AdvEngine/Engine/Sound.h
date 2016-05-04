#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <map>

#ifndef DISABLE_SOUND
#ifdef WIN32
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#endif

#include <system/thread.h>
#include <util/activeobject.h>

#include "AdvDoc.h"
#include "ExecutionContext.h"

namespace adv{

class SoundPlayer;
class VideoPlayer;
class AdvDocument;
class CharacterObject;
class ExecutionContext;

class SoundEngine : CGE::ActiveObject{
public:
  enum PlayerFlags{
    PLAYER_DEFAULT=0,
    PLAYER_CREATE_ALWAYS=1,
    PLAYER_UNMANAGED=2,
    PLAYER_UNREALIZED=4,
  };
  ~SoundEngine();
  static void init() {mInstance = new SoundEngine();}
  static void deinit() {delete mInstance;}
  static SoundEngine* instance() {return mInstance;}
  void setData(AdvDocument* doc){mData = doc;}
  SoundPlayer* getSound(const std::string& name, int flags);
  SoundPlayer* getSound(const std::string& name, const std::string& effect, int flags);
  SoundPlayer* getSound(const std::string& name, bool effectEnabled, int flags);
  SoundPlayer* getMusic(const std::string& name);
  SoundPlayer* getMusic(const std::string& name, bool effectEnabled);
  VideoPlayer* getMovie(const std::string& name, bool isSwf);
  void update(unsigned time);
  void removeSpeaker(CharacterObject* chr);
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in, int version);
  void setMusicVolume(float volume);
  float getMusicVolume() {return mMusicVolume;}
  void setSpeechVolume(float volume);
  float getSpeechVolume() {return mSpeechVolume;}
  void setMasterVolume(float volume);
  void setEAXEffect(const std::string& effect);
  void setFadingTime(unsigned time) {mFadingTime = time;}
  unsigned getFadingTime() {return mFadingTime;}
#ifndef DISABLE_SOUND
  ALuint getEffectSlot() {return mEffectSlot;}
  ALuint* getFilters() { return mFilters; }
#endif
  bool isEffectEnabled() {return mCurrentEffect != "none";}
  void removeSoundPlayer(SoundPlayer* plyr);
  void setSpeedFactor(float speed);
  void reset();
  bool setDSPEffect(const std::string& effect);
protected:
  SoundEngine();
  SoundPlayer* createPlayer(const std::string& name, const DataBuffer& db, bool effectEnabled);
  VideoPlayer* createVideoPlayer(const std::string& name, const DataBuffer& db, bool isSwf);
  bool run();
  bool run(unsigned time);
  static SoundEngine* mInstance;
  AdvDocument* mData;
#ifndef DISABLE_SOUND
  ALCdevice* mDevice;
  ALCcontext* mContext;
  ALuint mEffectSlot;
  ALuint mEffect[6];
  ALuint mFilters[2];
#endif
  std::multimap<std::string, SoundPlayer*> mActiveSounds;
  SoundPlayer* mActiveMusic;
  VideoPlayer* mActiveVideo;
  float mMusicVolume;
  float mSpeechVolume;
  std::string mCurrentEffect;
  bool mIsDSPEffect;
  unsigned mFadingTime;
  float mSpeedFactor;
  CGE::Mutex mMutex;
};

}

#endif
