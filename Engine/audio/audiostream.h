#ifndef CGE_AUDIOSTREAM_H
#define CGE_AUDIOSTREAM_H

#include <cge_export.h>

#include <cstddef>
#include <memory>

namespace CGE{

class AudioEffect;

//! A stream of PCM that the application decodes itself (Audio::createStream)
/*! Modelled on guit3's CTadsAudioDevice: open the stream, write the
    samples as they are decoded, then drain it (the natural end: what is
    queued plays out) or halt it (cancelled: it stops at once), and close
    it. A stream can be opened again with another format. It queues up to a
    second of audio and starts playing once a quarter of it is there (or at
    drain), so the decoder keeps ahead; when it runs dry, it plays silence.

    The stream is a node of the engine's mixer, not a device of its own, so
    it mixes with everything else and goes through the volumes. Except for
    setVolume, which any thread may call, call it from one thread at a time,
    typically a decoder's own thread. write and drain wait for the playback;
    on the browser's main thread, which also runs the playback, they queue
    what fits and return instead (single-threaded web pages use tryWrite and
    getQueuedBytes). Delete it before the Audio shuts down. */
class CGE_API AudioStream{
public:
  ~AudioStream();
  AudioStream(const AudioStream&) = delete;
  AudioStream& operator=(const AudioStream&) = delete;

  //! opens the stream for interleaved PCM in the machine's byte order
  /*! bitsPerSample: 8 (unsigned), 16 (signed) or 32 (float). Closes an open
      stream first. False if the format isn't supported. */
  bool open(int sampleRate, int bitsPerSample, int channels);
  //! queues samples; waits until all are queued, or until halt
  void write(const void* data, size_t bytes);
  //! queues as many samples as fit now and returns their bytes
  size_t tryWrite(const void* data, size_t bytes);
  //! the bytes queued and not played yet
  size_t getQueuedBytes() const;
  //! waits until everything queued has played, then stops
  void drain();
  //! marks the end without waiting: what is queued plays out, even less than the start needs
  void finish();
  //! stops at once and drops what is queued; write queues nothing until the next open
  void halt();
  //! closes the stream
  void close();
  bool isOpen() const;
  //! the volume, a gain; from any thread
  void setVolume(float volume);
  //! sends the stream through an effect, or directly to its group with null
  void setEffect(AudioEffect* effect);
  //! how often it ran dry while playing (for diagnostics)
  unsigned getUnderruns() const;
  //! the sample frames the mixer has taken from the queue since open (a clock for what plays)
  unsigned long long getPlayedFrames() const;

  struct Impl;
private:
  friend class Audio;
  explicit AudioStream(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
};

}

#endif
