#ifndef SOUND_PLAYER_H
#define SOUND_PLAYER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "AdvDoc.h"

namespace CGE{
class AudioPlayer;
}

namespace adv{

class CharacterObject;
class ExecutionContext;

//! a sound or a piece of music, decoded while it plays by the engine's audio (CGE::AudioPlayer)
class SoundPlayer{
public:
  SoundPlayer(const std::string& name, bool effectEnabled);
  ~SoundPlayer();
  //! the file to play (WAV, MP3, FLAC or Ogg Vorbis), copied
  void setStream(const DataBuffer& buffer);
  //! creates the engine's player; false if the file can't be decoded
  bool realize();
  void play(bool looping);
  void stop();
  //! goes to time (ms)
  void seek(int time);
  void setVolume(float volume);
  //! the sound's fades; false once it ended and is to be deleted
  bool update(unsigned time);
  void setSpeaker(CharacterObject* chr) {mSpeaker = chr;}
  CharacterObject* getSpeaker() {return mSpeaker;}
  void setSuspensionScript(ExecutionContext* ctx);
  void setSpokenString(/*FontRenderer::String*/void* string) {mSpokenString = string;}
  void fadeVolume(float from, float to, unsigned time);
  bool isFading() {return mFadeDuration != 0;}
  std::string getName() {return mName;}
  bool hasEffect() {return mEffectEnabled;}
  bool isLooping() {return mLooping;}
  bool isPlaying() {return mPlay;}
  //! unmanaged players (e.g. walk sounds) stay after their end
  void setAutoDelete(bool del) {mAutoDelete = del;}
  bool hasAutoDeletion() {return mAutoDelete;}
  //! the game's speed: a factor of speed and pitch
  void setSpeed(float factor);
protected:
  bool fadeUpdate(unsigned time);
  std::vector<uint8_t> mFile;
  std::unique_ptr<CGE::AudioPlayer> mPlayer;
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
  float mVolume;
  bool mLooping;
  bool mPlay;
  bool mAutoDelete;
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

#endif
