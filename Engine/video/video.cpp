// Plays a video: the sound goes to an AudioStream half a second ahead, and
// the frames the mixer has taken from it are the clock for the pictures.

#include "video.h"

#include <algorithm>

#include <audio/audio.h>
#include <audio/audiostream.h>
#include <io/Tracing.h>

using namespace CGE;

TR_CHANNEL(CGE_Video);

namespace{

//! how far the sound is queued ahead, in seconds
const double AUDIO_AHEAD = 0.5;
//! how long the sound may stand still before the pictures go on without it
const double AUDIO_STALL = 0.5;
//! at most this much of one update counts towards a stall (a long frame isn't one)
const double MAX_STALL_STEP = 0.1;

}

struct Video::Impl{
  std::unique_ptr<VideoDecoder> decoder;
  Audio* audio = nullptr;
  AudioGroup group = AudioGroup::Sound;
  std::unique_ptr<AudioStream> stream;
  AudioEffect* effect = nullptr;
  float volume = 1;
  //the sound decoded and not queued yet
  std::vector<float> pending;
  size_t pendingPos = 0;
  std::vector<unsigned char> pixels;
  unsigned frames = 0;
  bool playing = false;
  bool played = false;
  bool picturesEnded = false;
  bool soundEnded = false;
  //the sound is fed and is the clock
  bool sound = false;
  double clock = 0;
  double nextFrame = 0;
  double stall = 0;
  unsigned long long lastPlayed = 0;

  void feedSound();
};

void Video::Impl::feedSound(){
  if (!sound || soundEnded)
    return;
  size_t ahead = (size_t)(AUDIO_AHEAD*decoder->getSampleRate())*decoder->getChannels()*sizeof(float);
  while (stream->getQueuedBytes() < ahead){
    if (pendingPos >= pending.size()){
      pending.clear();
      pendingPos = 0;
      if (!decoder->decodeAudio(pending)){
        soundEnded = true;
        //the rest plays, however short
        stream->finish();
        return;
      }
      continue;
    }
    size_t bytes = (pending.size()-pendingPos)*sizeof(float);
    size_t written = stream->tryWrite(&pending[pendingPos], bytes);
    pendingPos += written/sizeof(float);
    if (written < bytes)
      return;
  }
}

Video::Video(std::unique_ptr<Impl> impl) : impl_(std::move(impl)){
}

Video::~Video(){
  stop();
}

std::unique_ptr<Video> Video::open(std::vector<unsigned char> data, Audio* audio, AudioGroup group){
  auto decoder = VideoDecoder::open(std::move(data));
  if (!decoder)
    return nullptr;
  auto impl = std::make_unique<Impl>();
  impl->decoder = std::move(decoder);
  impl->audio = audio;
  impl->group = group;
  impl->pixels.resize((size_t)impl->decoder->getWidth()*impl->decoder->getHeight()*4);
  return std::unique_ptr<Video>(new Video(std::move(impl)));
}

int Video::getWidth() const{
  return impl_->decoder->getWidth();
}

int Video::getHeight() const{
  return impl_->decoder->getHeight();
}

void Video::play(){
  TR_USE(CGE_Video);
  Impl& v = *impl_;
  stop();
  if (v.played)
    v.decoder->rewind();
  v.played = true;
  v.playing = true;
  v.picturesEnded = false;
  v.soundEnded = false;
  v.clock = 0;
  v.nextFrame = 0;
  v.stall = 0;
  v.lastPlayed = 0;
  v.pending.clear();
  v.pendingPos = 0;
  v.sound = false;
  if (v.audio && v.decoder->hasAudio()){
    if (!v.stream)
      v.stream = v.audio->createStream(v.group);
    if (v.stream && v.stream->open(v.decoder->getSampleRate(), 32, v.decoder->getChannels())){
      v.stream->setVolume(v.volume);
      v.stream->setEffect(v.effect);
      v.sound = true;
    }
    else
      TR_WARN("The video's sound can't be played");
  }
  v.feedSound();
}

void Video::stop(){
  Impl& v = *impl_;
  v.playing = false;
  v.sound = false;
  if (v.stream){
    v.stream->halt();
    v.stream->close();
  }
}

bool Video::update(double seconds){
  TR_USE(CGE_Video);
  Impl& v = *impl_;
  if (!v.playing)
    return false;
  v.feedSound();
  if (v.sound){
    unsigned long long played = v.stream->getPlayedFrames();
    if (played != v.lastPlayed){
      v.lastPlayed = played;
      v.stall = 0;
      v.clock = (double)played/v.decoder->getSampleRate();
    }
    else if (v.soundEnded && v.stream->getQueuedBytes() == 0){
      //the sound is over; longer pictures go on by the time passed
      v.sound = false;
    }
    else{
      v.stall += std::min(seconds, MAX_STALL_STEP);
      if (v.stall > AUDIO_STALL){
        //no output (e.g. a web page before its first click): the pictures don't wait for it
        TR_WARN("The video's sound doesn't play; the video goes on without it");
        v.sound = false;
        v.stream->halt();
      }
    }
  }
  else
    v.clock += seconds;

  //the pictures that are due; only the last one is converted
  bool fresh = false;
  double frameTime = v.decoder->getFrameRate() > 0 ? 1/v.decoder->getFrameRate() : 0;
  while (!v.picturesEnded && v.clock >= v.nextFrame){
    double time;
    if (!v.decoder->decodeFrame(&time)){
      v.picturesEnded = true;
      break;
    }
    fresh = true;
    v.nextFrame = time + frameTime;
  }
  if (fresh){
    v.decoder->convertFrame(v.pixels.data());
    ++v.frames;
  }

  bool soundPlaying = v.sound && !(v.soundEnded && v.stream->getQueuedBytes() == 0);
  if (v.picturesEnded && !soundPlaying && v.clock >= v.nextFrame)
    v.playing = false;
  return v.playing;
}

bool Video::isPlaying() const{
  return impl_->playing;
}

double Video::getTime() const{
  return impl_->clock;
}

const unsigned char* Video::getPixels() const{
  return impl_->frames > 0 ? impl_->pixels.data() : nullptr;
}

unsigned Video::getFrameCount() const{
  return impl_->frames;
}

void Video::setVolume(float volume){
  impl_->volume = volume;
  if (impl_->stream)
    impl_->stream->setVolume(volume);
}

void Video::setEffect(AudioEffect* effect){
  impl_->effect = effect;
  if (impl_->stream)
    impl_->stream->setEffect(effect);
}
