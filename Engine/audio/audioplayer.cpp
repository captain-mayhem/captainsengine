#include "audioplayer.h"
#include "audio_internal.h"

#include <algorithm>

using namespace CGE;

AudioPlayer::Impl::~Impl(){
  if (soundInit)
    ma_sound_uninit(&sound);
  if (decoderInit)
    ma_decoder_uninit(&decoder);
}

AudioPlayer::AudioPlayer(std::unique_ptr<Impl> impl) : impl_(std::move(impl)){
}

AudioPlayer::~AudioPlayer(){
}

void AudioPlayer::play(bool loop){
  ma_sound_set_looping(&impl_->sound, loop ? MA_TRUE : MA_FALSE);
  //starting at the end rewinds
  ma_sound_start(&impl_->sound);
}

void AudioPlayer::stop(){
  ma_sound_stop(&impl_->sound);
  //done by the mixer before it reads again
  ma_sound_seek_to_pcm_frame(&impl_->sound, 0);
}

void AudioPlayer::pause(){
  ma_sound_stop(&impl_->sound);
}

bool AudioPlayer::isPlaying() const{
  return ma_sound_is_playing(&impl_->sound) != MA_FALSE;
}

bool AudioPlayer::isLooping() const{
  return ma_sound_is_looping(&impl_->sound) != MA_FALSE;
}

void AudioPlayer::setVolume(float volume){
  ma_sound_set_volume(&impl_->sound, std::max(volume, 0.0f));
}

float AudioPlayer::getVolume() const{
  return ma_sound_get_volume(&impl_->sound);
}

void AudioPlayer::setPitch(float pitch){
  ma_sound_set_pitch(&impl_->sound, std::max(pitch, 0.01f));
}

void AudioPlayer::setEffect(AudioEffect* effect){
  ma_node* target = effect ? AudioInternal::effectNode(*effect) : AudioInternal::groupNode(*impl_->audio, impl_->group);
  ma_node_attach_output_bus(&impl_->sound, 0, target, 0);
}

double AudioPlayer::getLength() const{
  return (double)impl_->length/impl_->sampleRate;
}

double AudioPlayer::getPosition() const{
  ma_uint64 cursor = 0;
  if (ma_sound_get_cursor_in_pcm_frames(&impl_->sound, &cursor) != MA_SUCCESS)
    return 0;
  return (double)cursor/impl_->sampleRate;
}

void AudioPlayer::seek(double seconds){
  ma_sound_seek_to_pcm_frame(&impl_->sound, (ma_uint64)(std::max(seconds, 0.0)*impl_->sampleRate));
}
