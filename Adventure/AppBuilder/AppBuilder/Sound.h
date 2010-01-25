#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <map>
#include <al.h>
#include <alc.h>

class SoundPlayer;
class AdvDocument;

class SoundEngine{
public:
  ~SoundEngine();
  static void init() {mInstance = new SoundEngine();}
  static void deinit() {delete mInstance;}
  static SoundEngine* instance() {return mInstance;}
  void setData(AdvDocument* doc){mData = doc;}
  SoundPlayer* getSound(const std::string& name);
  void update();
protected:
  SoundEngine();
  static SoundEngine* mInstance;
  AdvDocument* mData;
  ALCdevice* mDevice;
  ALCcontext* mContext;
  std::map<std::string, SoundPlayer*> mActiveSounds;
};

class SoundPlayer{
public:
  SoundPlayer(ALuint buffer);
  ~SoundPlayer();
  void play(bool looping);
  void stop();
  void setVolume(float volume);
  bool update();
protected:
  ALuint mBuffer;
  ALuint mSource;
};

#endif
