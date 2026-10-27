#ifndef CGE_VIDEO_H
#define CGE_VIDEO_H

#include <cge_export.h>

#include <memory>
#include <vector>

#include "videodecoder.h"

namespace CGE{

class Audio;
class AudioEffect;
class AudioStream;
enum class AudioGroup;

//! Plays a video: its sound through an AudioStream, its pictures timed by the sound
/*! The application calls update every frame and shows getPixels (RGBA8)
    when getFrameCount changes, e.g. in a texture of its own. The sound is
    the clock: a picture is shown when the sound has played up to its time,
    and late pictures are skipped. Without sound (none in the file, no audio
    device, or a sound that doesn't play, as on a web page before the first
    click) the time passed to update is the clock.

    Everything is decoded on the calling thread, in update. Call it from the
    engine's thread, and delete it before the Audio shuts down. */
class CGE_API Video{
public:
  //! opens a video in memory (see VideoDecoder); null if it can't be decoded
  /*! audio may be null for a video without sound. */
  static std::unique_ptr<Video> open(std::vector<unsigned char> data, Audio* audio, AudioGroup group);
  ~Video();
  Video(const Video&) = delete;
  Video& operator=(const Video&) = delete;

  int getWidth() const;
  int getHeight() const;
  //! starts playing from the beginning
  void play();
  //! stops; the last picture stays
  void stop();
  //! decodes and plays what is due after seconds more; false once it ended or was stopped
  bool update(double seconds);
  bool isPlaying() const;
  //! the time played in seconds
  double getTime() const;
  //! the picture to show, RGBA8, getWidth()*getHeight()*4 bytes; null before the first
  const unsigned char* getPixels() const;
  //! counts the pictures shown, so a new one can be told from the last
  unsigned getFrameCount() const;
  //! the sound's volume, a gain
  void setVolume(float volume);
  //! sends the sound through an effect, or directly to its group with null
  void setEffect(AudioEffect* effect);

  struct Impl;
private:
  explicit Video(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
};

}

#endif
