#ifndef CGE_AUDIOPLAYER_H
#define CGE_AUDIOPLAYER_H

#include <cge_export.h>

#include <memory>

namespace CGE{

class AudioEffect;

//! Plays one sound file from memory, decoded while it plays (Audio::createPlayer)
/*! For music and long sounds, and for applications that control each sound
    themselves (volume, pitch, looping, the end). Starts stopped. Call it
    from the engine's thread, and delete it before the Audio shuts down. */
class CGE_API AudioPlayer{
public:
  ~AudioPlayer();
  AudioPlayer(const AudioPlayer&) = delete;
  AudioPlayer& operator=(const AudioPlayer&) = delete;

  //! plays from where it is (from the start after stop or the end)
  void play(bool loop = false);
  //! stops and goes back to the start
  void stop();
  //! stops where it is; play goes on from there
  void pause();
  //! is it playing (false after the end of a sound that doesn't loop)
  bool isPlaying() const;
  bool isLooping() const;
  //! the volume, a gain
  void setVolume(float volume);
  float getVolume() const;
  //! a factor of speed and pitch
  void setPitch(float pitch);
  //! sends the sound through an effect, or directly to its group with null
  /*! The effect must stay until the player goes or is sent elsewhere. */
  void setEffect(AudioEffect* effect);
  //! the length and the position in seconds (the length is 0 if the file doesn't tell)
  double getLength() const;
  double getPosition() const;
  //! goes to a position in seconds
  void seek(double seconds);

  struct Impl;
private:
  friend class Audio;
  explicit AudioPlayer(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
};

}

#endif
