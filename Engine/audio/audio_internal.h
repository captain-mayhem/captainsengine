#ifndef CGE_AUDIO_INTERNAL_H
#define CGE_AUDIO_INTERNAL_H

// The audio module's miniaudio side, shared by its source files; not
// installed, so miniaudio stays out of the public headers.

//miniaudio includes windows.h
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <miniaudio.h>

#include <system/types.h>

#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "audio.h"
#include "audioeffect.h"
#include "audioplayer.h"

namespace CGE{

namespace AudioInternal{

//! a decoder config for 32-bit float output, with Ogg Vorbis (libvorbis) next to miniaudio's own decoders
ma_decoder_config decoderConfig();

//! a whole file decoded into memory
struct DecodedSound{
  std::vector<float> frames;
  ma_uint32 channels = 0;
  ma_uint32 sampleRate = 0;
  ma_uint64 frameCount = 0;
};

//! decodes a file's bytes completely; false if no decoder takes them
bool decodeAll(const void* data, size_t size, DecodedSound& sound);

//! the node the players and streams of a group (and of an effect) are attached to
ma_node* groupNode(Audio::Impl& audio, AudioGroup group);

//! the node an effect takes its input at
ma_node* effectNode(AudioEffect& effect);

//! the module's access to the classes' implementations
struct Access{
  static AudioEffect::Impl& impl(AudioEffect& effect) {return *effect.impl_;}
};

}

struct Audio::Impl{
  //! is engine initialised
  bool open = false;
  bool device = false;
  ma_engine engine;
  ma_log log;
  bool logInit = false;
  ma_sound_group groups[2];

  //! the sounds of loadSound
  std::map<SoundId, std::shared_ptr<AudioInternal::DecodedSound>> sounds;
  SoundId nextSound = 1;
  //! one playSound
  struct Voice{
    std::shared_ptr<AudioInternal::DecodedSound> sound;
    ma_audio_buffer_ref buffer;
    ma_sound node;
  };
  std::list<std::unique_ptr<Voice>> voices;

  //! the music, and the pieces fading out
  struct Music{
    std::string path;
    std::unique_ptr<AudioPlayer> player;
  };
  std::unique_ptr<Music> music;
  std::list<std::unique_ptr<Music>> fadingMusic;

  ~Impl();
  void stopVoice(Voice& voice);
};

struct AudioPlayer::Impl{
  Audio::Impl* audio = nullptr;
  AudioGroup group = AudioGroup::Sound;
  std::vector<uint8> file;
  ma_decoder decoder;
  bool decoderInit = false;
  ma_sound sound;
  bool soundInit = false;
  //the decoder's rate and the file's length in its frames (0 if unknown)
  ma_uint32 sampleRate = 0;
  ma_uint64 length = 0;

  ~Impl();
};

}

#endif
