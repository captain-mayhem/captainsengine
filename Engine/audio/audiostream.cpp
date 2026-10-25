// The PCM stream: a ring buffer (miniaudio's ma_pcm_rb, lock-free for one
// writer and the mixer as reader) read by a data source that a sound of the
// engine plays; it gives silence while the buffer is empty.

#include "audiostream.h"
#include "audio_internal.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>

#include <io/Tracing.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/threading.h>
#endif

using namespace CGE;

TR_CHANNEL(CGE_AudioStream);

namespace{

struct StreamSource{
  ma_data_source_base base;
  AudioStream::Impl* stream;
};

//! can this thread wait for the playback? Not the browser's main thread, which also runs it
bool canWait(){
#ifdef __EMSCRIPTEN__
  return !emscripten_is_main_runtime_thread();
#else
  return true;
#endif
}

}

struct AudioStream::Impl{
  Audio::Impl* audio = nullptr;
  AudioGroup group = AudioGroup::Sound;
  AudioEffect* effect = nullptr;
  StreamSource source;
  bool sourceInit = false;
  ma_pcm_rb buffer;
  bool bufferInit = false;
  ma_sound sound;
  bool soundInit = false;
  ma_format format = ma_format_s16;
  ma_uint32 channels = 2;
  ma_uint32 sampleRate = 44100;
  ma_uint32 bytesPerFrame = 4;
  //it plays once this much is queued
  ma_uint32 startFrames = 1;
  std::atomic<bool> started{false};
  std::atomic<bool> halted{false};
  std::atomic<bool> draining{false};
  std::atomic<unsigned> underruns{0};
  //guards the sound against setVolume from other threads
  std::mutex mutex;
  float volume = 1;
  bool warned = false;

  void closeStream();
};

namespace{

ma_result streamRead(ma_data_source* source, void* out, ma_uint64 frameCount, ma_uint64* framesRead){
  AudioStream::Impl& s = *((StreamSource*)source)->stream;
  ma_uint64 done = 0;
  if (!s.halted){
    if (!s.started){
      //a quarter of the buffer, a full buffer, or everything there is
      if (ma_pcm_rb_available_read(&s.buffer) >= s.startFrames || ma_pcm_rb_available_write(&s.buffer) == 0 || s.draining)
        s.started = true;
    }
    if (s.started){
      while (done < frameCount){
        ma_uint32 count = (ma_uint32)std::min<ma_uint64>(frameCount-done, 0xffffffffu);
        void* src = nullptr;
        if (ma_pcm_rb_acquire_read(&s.buffer, &count, &src) != MA_SUCCESS || count == 0)
          break;
        memcpy((char*)out + done*s.bytesPerFrame, src, (size_t)count*s.bytesPerFrame);
        ma_pcm_rb_commit_read(&s.buffer, count);
        done += count;
      }
      if (done < frameCount && !s.draining)
        ++s.underruns;
    }
  }
  ma_silence_pcm_frames((char*)out + done*s.bytesPerFrame, frameCount-done, s.format, s.channels);
  //it never ends by itself
  *framesRead = frameCount;
  return MA_SUCCESS;
}

ma_result streamFormat(ma_data_source* source, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap){
  AudioStream::Impl& s = *((StreamSource*)source)->stream;
  if (format)
    *format = s.format;
  if (channels)
    *channels = s.channels;
  if (sampleRate)
    *sampleRate = s.sampleRate;
  if (channelMap)
    ma_channel_map_init_standard(ma_standard_channel_map_default, channelMap, channelMapCap, s.channels);
  return MA_SUCCESS;
}

ma_data_source_vtable streamVtable = {
  streamRead,
  nullptr,
  streamFormat,
  nullptr,
  nullptr,
  nullptr,
  0
};

}

void AudioStream::Impl::closeStream(){
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (soundInit)
      ma_sound_uninit(&sound);
    soundInit = false;
  }
  if (sourceInit)
    ma_data_source_uninit(&source.base);
  sourceInit = false;
  if (bufferInit)
    ma_pcm_rb_uninit(&buffer);
  bufferInit = false;
}

AudioStream::AudioStream(std::unique_ptr<Impl> impl) : impl_(std::move(impl)){
}

AudioStream::~AudioStream(){
  close();
}

bool AudioStream::open(int sampleRate, int bitsPerSample, int channels){
  TR_USE(CGE_AudioStream);
  close();
  Impl& s = *impl_;
  switch (bitsPerSample){
    case 8: s.format = ma_format_u8; break;
    case 16: s.format = ma_format_s16; break;
    case 32: s.format = ma_format_f32; break;
    default:
      TR_ERROR("Streams have 8, 16 or 32 bits per sample, not %i", bitsPerSample);
      return false;
  }
  if (sampleRate <= 0 || channels <= 0 || channels > MA_MAX_CHANNELS){
    TR_ERROR("Unsupported stream format: %i Hz, %i channels", sampleRate, channels);
    return false;
  }
  s.channels = (ma_uint32)channels;
  s.sampleRate = (ma_uint32)sampleRate;
  s.bytesPerFrame = ma_get_bytes_per_frame(s.format, s.channels);
  //a second of audio, playing once a quarter of it is there (as guit3)
  s.startFrames = std::max<ma_uint32>(s.sampleRate/4, 1);
  s.started = false;
  s.halted = false;
  s.draining = false;
  s.underruns = 0;
  s.warned = false;
  if (ma_pcm_rb_init(s.format, s.channels, s.sampleRate, nullptr, nullptr, &s.buffer) != MA_SUCCESS)
    return false;
  s.bufferInit = true;
  ma_data_source_config config = ma_data_source_config_init();
  config.vtable = &streamVtable;
  s.source.stream = &s;
  if (ma_data_source_init(&config, &s.source.base) != MA_SUCCESS){
    close();
    return false;
  }
  s.sourceInit = true;
  ma_sound_group* group = &s.audio->groups[s.group == AudioGroup::Music ? 1 : 0];
  if (ma_sound_init_from_data_source(&s.audio->engine, &s.source, MA_SOUND_FLAG_NO_SPATIALIZATION, group, &s.sound) != MA_SUCCESS){
    TR_ERROR("The stream can't be played");
    close();
    return false;
  }
  std::lock_guard<std::mutex> lock(s.mutex);
  s.soundInit = true;
  if (s.effect)
    ma_node_attach_output_bus(&s.sound, 0, AudioInternal::effectNode(*s.effect), 0);
  ma_sound_set_volume(&s.sound, s.volume);
  //silent until enough is queued
  ma_sound_start(&s.sound);
  return true;
}

size_t AudioStream::tryWrite(const void* data, size_t bytes){
  Impl& s = *impl_;
  if (!s.bufferInit || s.halted)
    return 0;
  const char* src = (const char*)data;
  size_t written = 0;
  while (bytes-written >= s.bytesPerFrame){
    ma_uint32 count = (ma_uint32)std::min<size_t>((bytes-written)/s.bytesPerFrame, 0xffffffffu);
    void* dst = nullptr;
    if (ma_pcm_rb_acquire_write(&s.buffer, &count, &dst) != MA_SUCCESS || count == 0)
      break;
    memcpy(dst, src+written, (size_t)count*s.bytesPerFrame);
    ma_pcm_rb_commit_write(&s.buffer, count);
    written += (size_t)count*s.bytesPerFrame;
  }
  return written;
}

void AudioStream::write(const void* data, size_t bytes){
  TR_USE(CGE_AudioStream);
  Impl& s = *impl_;
  const char* src = (const char*)data;
  while (!s.halted && s.bufferInit){
    size_t written = tryWrite(src, bytes);
    src += written;
    bytes -= written;
    if (bytes < s.bytesPerFrame)
      return;
    if (!canWait()){
      if (!s.warned)
        TR_ERROR("AudioStream::write can't wait on the browser's main thread: samples are dropped");
      s.warned = true;
      return;
    }
    //full: the playback makes room
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

size_t AudioStream::getQueuedBytes() const{
  if (!impl_->bufferInit)
    return 0;
  return (size_t)ma_pcm_rb_available_read(&impl_->buffer)*impl_->bytesPerFrame;
}

void AudioStream::drain(){
  Impl& s = *impl_;
  if (!s.soundInit || s.halted)
    return;
  //a sound shorter than the start threshold plays now
  s.draining = true;
  if (!canWait())
    return;
  while (!s.halted && ma_pcm_rb_available_read(&s.buffer) > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  //what the mixer read is still in the device's buffers
  ma_device* device = ma_engine_get_device(&s.audio->engine);
  ma_uint32 latency = device ? device->playback.internalPeriodSizeInFrames*device->playback.internalPeriods : 0;
  ma_uint32 rate = ma_engine_get_sample_rate(&s.audio->engine);
  std::this_thread::sleep_for(std::chrono::milliseconds(20 + (rate > 0 ? 1000*latency/rate : 0)));
  std::lock_guard<std::mutex> lock(s.mutex);
  if (s.soundInit)
    ma_sound_stop(&s.sound);
}

void AudioStream::halt(){
  Impl& s = *impl_;
  s.halted = true;
  std::lock_guard<std::mutex> lock(s.mutex);
  if (s.soundInit)
    ma_sound_stop(&s.sound);
}

void AudioStream::close(){
  impl_->closeStream();
}

bool AudioStream::isOpen() const{
  return impl_->soundInit;
}

void AudioStream::setVolume(float volume){
  Impl& s = *impl_;
  std::lock_guard<std::mutex> lock(s.mutex);
  s.volume = std::max(volume, 0.0f);
  if (s.soundInit)
    ma_sound_set_volume(&s.sound, s.volume);
}

void AudioStream::setEffect(AudioEffect* effect){
  Impl& s = *impl_;
  s.effect = effect;
  std::lock_guard<std::mutex> lock(s.mutex);
  if (s.soundInit){
    ma_node* target = effect ? AudioInternal::effectNode(*effect) : AudioInternal::groupNode(*s.audio, s.group);
    ma_node_attach_output_bus(&s.sound, 0, target, 0);
  }
}

unsigned AudioStream::getUnderruns() const{
  return impl_->underruns;
}

std::unique_ptr<AudioStream> Audio::createStream(AudioGroup group){
  if (!impl_->open)
    return nullptr;
  auto impl = std::make_unique<AudioStream::Impl>();
  impl->audio = impl_.get();
  impl->group = group;
  return std::unique_ptr<AudioStream>(new AudioStream(std::move(impl)));
}
