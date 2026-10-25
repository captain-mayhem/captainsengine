// first: it includes miniaudio, which includes windows.h without min and max
#include "audio_internal.h"
#include "audio.h"
#include "audioeffect.h"
#include "audiostream.h"

#include <algorithm>
#include <cstring>

#include <io/Tracing.h>
#include <platform/assets.h>

using namespace CGE;

TR_CHANNEL(CGE_Audio);

namespace{

//! the most sounds of playSound at once
const size_t MAX_VOICES = 64;
//! the mixer's sample rate without a device
const int DEFAULT_SAMPLE_RATE = 48000;

//! miniaudio's log messages as traces
void logMessage(void* userData, ma_uint32 level, const char* message){
  TR_USE(CGE_Audio);
  std::string text(message);
  while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
    text.pop_back();
  switch (level){
    case MA_LOG_LEVEL_ERROR: TR_ERROR("miniaudio: %s", text.c_str()); break;
    case MA_LOG_LEVEL_WARNING: TR_WARN("miniaudio: %s", text.c_str()); break;
    case MA_LOG_LEVEL_INFO: TR_INFO("miniaudio: %s", text.c_str()); break;
    default: TR_DEBUG("miniaudio: %s", text.c_str()); break;
  }
}

int groupIndex(AudioGroup group){
  return group == AudioGroup::Music ? 1 : 0;
}

}

ma_node* AudioInternal::groupNode(Audio::Impl& audio, AudioGroup group){
  return (ma_node*)&audio.groups[groupIndex(group)];
}

Audio::Impl::~Impl(){
}

void Audio::Impl::stopVoice(Voice& voice){
  ma_sound_uninit(&voice.node);
  ma_audio_buffer_ref_uninit(&voice.buffer);
}

Audio::Audio() : impl_(std::make_unique<Impl>()){
}

Audio::~Audio(){
  shutdown();
}

bool Audio::init(const Desc& desc){
  TR_USE(CGE_Audio);
  shutdown();
  Impl& a = *impl_;
  if (ma_log_init(nullptr, &a.log) == MA_SUCCESS){
    a.logInit = true;
    ma_log_register_callback(&a.log, ma_log_callback_init(logMessage, nullptr));
  }
  ma_engine_config config = ma_engine_config_init();
  if (a.logInit)
    config.pLog = &a.log;
  //the effects work in stereo; the device maps it to its channels
  config.channels = 2;
  config.sampleRate = desc.sampleRate > 0 ? (ma_uint32)desc.sampleRate : 0;
  config.noDevice = desc.device ? MA_FALSE : MA_TRUE;
  if (!desc.device && config.sampleRate == 0)
    config.sampleRate = DEFAULT_SAMPLE_RATE;
  ma_result result = ma_engine_init(&config, &a.engine);
  if (result != MA_SUCCESS){
    TR_WARN("No audio: the output can't be opened (%s)", ma_result_description(result));
    if (a.logInit)
      ma_log_uninit(&a.log);
    a.logInit = false;
    return false;
  }
  for (int i = 0; i < 2; ++i)
    ma_sound_group_init(&a.engine, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, &a.groups[i]);
  a.open = true;
  a.device = desc.device;
  if (a.device){
    ma_device* device = ma_engine_get_device(&a.engine);
    TR_INFO("Audio output: %s, %s, %u Hz", ma_get_backend_name(device->pContext->backend),
      device->playback.name, ma_engine_get_sample_rate(&a.engine));
    //miniaudio's last resort plays in time, so sounds still end
    if (device->pContext->backend == ma_backend_null)
      TR_WARN("No audio backend works; nothing is heard");
  }
  return true;
}

void Audio::shutdown(){
  Impl& a = *impl_;
  if (!a.open)
    return;
  a.music.reset();
  a.fadingMusic.clear();
  for (auto& voice : a.voices)
    a.stopVoice(*voice);
  a.voices.clear();
  a.sounds.clear();
  for (int i = 0; i < 2; ++i)
    ma_sound_group_uninit(&a.groups[i]);
  ma_engine_uninit(&a.engine);
  if (a.logInit)
    ma_log_uninit(&a.log);
  a.logInit = false;
  a.open = false;
}

bool Audio::isOpen() const{
  return impl_->open;
}

int Audio::getSampleRate() const{
  return impl_->open ? (int)ma_engine_get_sample_rate(&impl_->engine) : 0;
}

std::string Audio::getBackendName() const{
  if (!impl_->open || !impl_->device)
    return std::string();
  ma_device* device = ma_engine_get_device(&impl_->engine);
  return ma_get_backend_name(device->pContext->backend);
}

void Audio::render(float* out, unsigned frames){
  if (!impl_->open || impl_->device){
    memset(out, 0, sizeof(float)*2*frames);
    return;
  }
  ma_engine_read_pcm_frames(&impl_->engine, out, frames, nullptr);
}

void Audio::pause(){
  if (impl_->open && impl_->device)
    ma_engine_stop(&impl_->engine);
}

void Audio::resume(){
  if (impl_->open && impl_->device)
    ma_engine_start(&impl_->engine);
}

void Audio::update(){
  Impl& a = *impl_;
  if (!a.open)
    return;
  for (auto it = a.voices.begin(); it != a.voices.end();){
    if (!ma_sound_is_playing(&(*it)->node)){
      a.stopVoice(**it);
      it = a.voices.erase(it);
    }
    else
      ++it;
  }
  a.fadingMusic.remove_if([](const std::unique_ptr<Impl::Music>& music){
    return !music->player->isPlaying();
  });
}

void Audio::setMasterVolume(float volume){
  if (impl_->open)
    ma_engine_set_volume(&impl_->engine, std::max(volume, 0.0f));
}

float Audio::getMasterVolume() const{
  return impl_->open ? ma_engine_get_volume(&impl_->engine) : 1.0f;
}

void Audio::setVolume(AudioGroup group, float volume){
  if (impl_->open)
    ma_sound_group_set_volume(&impl_->groups[groupIndex(group)], std::max(volume, 0.0f));
}

float Audio::getVolume(AudioGroup group) const{
  return impl_->open ? ma_sound_group_get_volume(&impl_->groups[groupIndex(group)]) : 1.0f;
}

SoundId Audio::loadSound(const std::string& path){
  TR_USE(CGE_Audio);
  if (!impl_->open)
    return 0;
  std::vector<uint8> data;
  if (!Assets::load(path, data)){
    TR_WARN("Sound %s not found", path.c_str());
    return 0;
  }
  SoundId id = loadSound(data.data(), data.size());
  if (id == 0)
    TR_WARN("Sound %s can't be decoded", path.c_str());
  return id;
}

SoundId Audio::loadSound(const void* data, size_t size){
  if (!impl_->open)
    return 0;
  auto sound = std::make_shared<AudioInternal::DecodedSound>();
  if (!AudioInternal::decodeAll(data, size, *sound))
    return 0;
  SoundId id = impl_->nextSound++;
  impl_->sounds[id] = sound;
  return id;
}

void Audio::unloadSound(SoundId id){
  //the voices keep their data
  impl_->sounds.erase(id);
}

void Audio::playSound(SoundId id, float volume, float pitch){
  TR_USE(CGE_Audio);
  Impl& a = *impl_;
  auto sound = a.sounds.find(id);
  if (!a.open || sound == a.sounds.end())
    return;
  if (a.voices.size() >= MAX_VOICES){
    a.stopVoice(*a.voices.front());
    a.voices.pop_front();
  }
  auto voice = std::make_unique<Impl::Voice>();
  voice->sound = sound->second;
  const AudioInternal::DecodedSound& data = *voice->sound;
  if (ma_audio_buffer_ref_init(ma_format_f32, data.channels, data.frames.data(), data.frameCount, &voice->buffer) != MA_SUCCESS)
    return;
  //the reference doesn't take the rate itself (miniaudio 0.11)
  voice->buffer.sampleRate = data.sampleRate;
  if (ma_sound_init_from_data_source(&a.engine, &voice->buffer, MA_SOUND_FLAG_NO_SPATIALIZATION, &a.groups[0], &voice->node) != MA_SUCCESS){
    TR_WARN("A sound can't be played");
    ma_audio_buffer_ref_uninit(&voice->buffer);
    return;
  }
  ma_sound_set_volume(&voice->node, std::max(volume, 0.0f));
  ma_sound_set_pitch(&voice->node, std::max(pitch, 0.01f));
  ma_sound_start(&voice->node);
  a.voices.push_back(std::move(voice));
}

void Audio::stopSounds(){
  for (auto& voice : impl_->voices)
    impl_->stopVoice(*voice);
  impl_->voices.clear();
}

int Audio::getPlayingSounds() const{
  int count = 0;
  for (const auto& voice : impl_->voices){
    if (ma_sound_is_playing(&voice->node))
      ++count;
  }
  return count;
}

bool Audio::playMusic(const std::string& path, bool loop, float fade){
  TR_USE(CGE_Audio);
  Impl& a = *impl_;
  if (!a.open)
    return false;
  if (a.music && a.music->path == path && a.music->player->isPlaying())
    return true;
  std::vector<uint8> data;
  if (!Assets::load(path, data)){
    TR_WARN("Music %s not found", path.c_str());
    return false;
  }
  auto music = std::make_unique<Impl::Music>();
  music->path = path;
  music->player = createPlayer(std::move(data), AudioGroup::Music);
  if (!music->player){
    TR_WARN("Music %s can't be decoded", path.c_str());
    return false;
  }
  stopMusic(fade);
  ma_uint64 ms = (ma_uint64)(std::max(fade, 0.0f)*1000);
  if (ms > 0)
    ma_sound_set_fade_in_milliseconds(&music->player->impl_->sound, 0, 1, ms);
  music->player->play(loop);
  a.music = std::move(music);
  return true;
}

void Audio::stopMusic(float fade){
  Impl& a = *impl_;
  if (!a.music)
    return;
  ma_uint64 ms = (ma_uint64)(std::max(fade, 0.0f)*1000);
  if (ms > 0 && a.music->player->isPlaying()){
    //it goes when the fade has stopped it (update)
    ma_sound_stop_with_fade_in_milliseconds(&a.music->player->impl_->sound, ms);
    a.fadingMusic.push_back(std::move(a.music));
  }
  a.music.reset();
}

bool Audio::isMusicPlaying() const{
  if (impl_->music && impl_->music->player->isPlaying())
    return true;
  for (const auto& music : impl_->fadingMusic){
    if (music->player->isPlaying())
      return true;
  }
  return false;
}

std::unique_ptr<AudioPlayer> Audio::createPlayer(std::vector<uint8_t> file, AudioGroup group){
  Impl& a = *impl_;
  if (!a.open || file.empty())
    return nullptr;
  auto impl = std::make_unique<AudioPlayer::Impl>();
  impl->audio = &a;
  impl->group = group;
  impl->file = std::move(file);
  ma_decoder_config config = AudioInternal::decoderConfig();
  if (ma_decoder_init_memory(impl->file.data(), impl->file.size(), &config, &impl->decoder) != MA_SUCCESS)
    return nullptr;
  impl->decoderInit = true;
  ma_format format;
  ma_uint32 channels;
  if (ma_decoder_get_data_format(&impl->decoder, &format, &channels, &impl->sampleRate, nullptr, 0) != MA_SUCCESS || impl->sampleRate == 0)
    return nullptr;
  //measured now, before the mixer reads the decoder (an MP3 may be scanned for it)
  if (ma_decoder_get_length_in_pcm_frames(&impl->decoder, &impl->length) != MA_SUCCESS)
    impl->length = 0;
  if (ma_sound_init_from_data_source(&a.engine, &impl->decoder, MA_SOUND_FLAG_NO_SPATIALIZATION, &a.groups[groupIndex(group)], &impl->sound) != MA_SUCCESS)
    return nullptr;
  impl->soundInit = true;
  return std::unique_ptr<AudioPlayer>(new AudioPlayer(std::move(impl)));
}
