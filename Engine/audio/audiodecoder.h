#ifndef CGE_AUDIODECODER_H
#define CGE_AUDIODECODER_H

#include <cge_export.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace CGE{

//! Decodes a sound file in memory into PCM (WAV, MP3, FLAC or Ogg Vorbis)
/*! For applications that play the samples themselves, typically through an
    AudioStream on a thread of their own. The same decoders as
    Audio::createPlayer, with the same formats: WAV in its PCM, float and
    compressed (ADPCM, A-law, mu-law) kinds, MPEG audio layers I to III,
    FLAC, and Ogg Vorbis. The samples come out as interleaved 32-bit float
    in the file's own rate and channels.

    It needs no Audio (it works while that is closed). Any thread may use a
    decoder, one at a time. */
class CGE_API AudioDecoder{
public:
  //! opens a file's bytes; null if no decoder takes them
  static std::unique_ptr<AudioDecoder> open(std::vector<uint8_t> file);

  ~AudioDecoder();
  AudioDecoder(const AudioDecoder&) = delete;
  AudioDecoder& operator=(const AudioDecoder&) = delete;

  //! the samples per second
  int getSampleRate() const;
  //! the channels, interleaved
  int getChannels() const;
  //! the length in sample frames, 0 if the file doesn't tell
  /*! An MP3 is read through for it (its frames' headers, not decoded); the
      position stays where it is. */
  unsigned long long getLength();
  //! decodes up to frames sample frames into samples (frames*getChannels() floats)
  /*! Returns the frames decoded: fewer only at the end, 0 after it. */
  size_t read(float* samples, size_t frames);
  //! goes to a sample frame; false if the file can't go there
  bool seek(unsigned long long frame);

  struct Impl;
private:
  explicit AudioDecoder(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
};

}

#endif
