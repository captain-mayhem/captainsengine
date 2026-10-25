#ifndef CGE_AUDIO_H
#define CGE_AUDIO_H

#include <cge_export.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace CGE{

class AudioEffect;
class AudioPlayer;
class AudioStream;

//! the mixer's groups, each with a volume of its own under the master volume
enum class AudioGroup{
  //! sound effects (Audio::playSound), and players and streams by default
  Sound,
  //! the music (Audio::playMusic)
  Music,
};

//! a sound loaded for Audio::playSound; 0 is none
typedef unsigned SoundId;

//! The engine's audio: one-shot sounds, music, players and streams, mixed by miniaudio
/*! The engine opens it at startup (unless engine.ini says audio = false) on
    the system's default output: WASAPI on Windows, PulseAudio or ALSA on
    Linux, AAudio or OpenSL ES on Android, and Web Audio in the browser. It
    stops while the engine is paused (in the background) and goes on with it.
    Files are WAV, MP3, FLAC or Ogg Vorbis, in the assets or in memory.

    Everything is mixed in 32-bit float stereo at the device's sample rate;
    the master volume and the groups' volumes apply on top of each sound's
    own volume. Volumes are linear gains (1 is unchanged, 0 silent).

    On the web, browsers only start audio after the first click or tap on the
    page; what plays before is silent (it isn't held back). The device is
    opened on the browser's main thread, which runs the engine in both web
    configurations, and is only touched from there: players, streams and
    effects are nodes of the mixer, so streams can be fed from other threads.

    Unless documented otherwise, call it from the engine's thread. Players,
    streams and effects must be deleted before it shuts down. Without an
    output device (or before init) it is closed: loading fails, playing does
    nothing, and the create functions return null. */
class CGE_API Audio{
public:
  //! how the mixer is opened
  struct Desc{
    //! output to the system's default device; false mixes only when render is called (for tests)
    bool device = true;
    //! the mixer's sample rate, 0 for the device's (48000 without a device)
    int sampleRate = 0;
  };

  Audio();
  ~Audio();
  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  //! opens the mixer; false if the device can't be opened
  bool init(const Desc& desc);
  bool init() {return init(Desc());}
  //! stops everything and closes the device; the sounds are unloaded
  void shutdown();
  //! is the mixer open
  bool isOpen() const;
  //! the mixer's sample rate, 0 while closed
  int getSampleRate() const;
  //! the name of the backend, e.g. "WASAPI" or "Web Audio"; empty without a device
  std::string getBackendName() const;
  //! mixes frames of interleaved stereo into out (only without a device)
  void render(float* out, unsigned frames);
  //! stops the output (the engine's pause), and starts it again
  void pause();
  void resume();
  //! frees the voices of sounds and music that ended; the engine calls it every frame
  void update();

  //! the volume of everything
  void setMasterVolume(float volume);
  float getMasterVolume() const;
  //! the volume of a group
  void setVolume(AudioGroup group, float volume);
  float getVolume(AudioGroup group) const;

  //! loads a sound for playSound from an asset, decoded into memory; 0 if it can't be read
  SoundId loadSound(const std::string& path);
  //! loads a sound from a file's bytes
  SoundId loadSound(const void* data, size_t size);
  //! unloads a sound; what plays of it goes on to its end
  void unloadSound(SoundId id);
  //! plays a sound once, in the Sound group
  /*! volume is a gain, pitch a factor of speed and pitch (2 is an octave
      higher and twice as fast). The same sound can play many times at once;
      at most 64 sounds play together, the oldest stops for a new one. */
  void playSound(SoundId id, float volume = 1, float pitch = 1);
  //! stops all sounds of playSound
  void stopSounds();
  //! how many sounds of playSound are playing
  int getPlayingSounds() const;

  //! plays music from an asset, decoded while it plays, in the Music group
  /*! A new piece fades in over fade seconds while the previous one fades
      out (a crossfade); the same piece again goes on playing. False if the
      file can't be read; the previous music then goes on. */
  bool playMusic(const std::string& path, bool loop = true, float fade = 0);
  //! stops the music, fading it out over fade seconds
  void stopMusic(float fade = 0);
  //! is music playing (also while it fades out)
  bool isMusicPlaying() const;

  //! a player for a file's bytes (WAV, MP3, FLAC or Ogg Vorbis), decoded while it plays; null if they can't be decoded
  std::unique_ptr<AudioPlayer> createPlayer(std::vector<uint8_t> file, AudioGroup group = AudioGroup::Sound);
  //! a stream for PCM that the application decodes itself
  std::unique_ptr<AudioStream> createStream(AudioGroup group = AudioGroup::Sound);
  //! an effect bus that players and streams can be sent through
  std::unique_ptr<AudioEffect> createEffect(AudioGroup group = AudioGroup::Sound);

  struct Impl;
private:
  std::unique_ptr<Impl> impl_;
};

}

#endif
