#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <al.h>
#include <alc.h>

class SoundEngine{
public:
  ~SoundEngine();
  static void init() {mInstance = new SoundEngine();}
  static void deinit() {delete mInstance;}
  static SoundEngine* instance() {return mInstance;}
protected:
  SoundEngine();
  static SoundEngine* mInstance;
  ALCdevice* mDevice;
  ALCcontext* mContext;
};

class SoundPlayer{
public:
  SoundPlayer(const std::string file);
  ~SoundPlayer();
  void play();
protected:
};

#endif
