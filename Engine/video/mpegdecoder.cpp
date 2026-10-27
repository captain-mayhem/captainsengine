// MPEG-1 video and MP2 sound in an MPEG program stream, decoded with pl_mpeg.

#include "video_internal.h"

#define PL_MPEG_IMPLEMENTATION
#include <pl_mpeg.h>

#include <io/Tracing.h>

using namespace CGE;

TR_CHANNEL(CGE_VideoMpeg);

namespace{

class MpegDecoder : public VideoDecoder{
public:
  explicit MpegDecoder(std::vector<unsigned char>& data) : data_(std::move(data)){
    //pl_mpeg reads the bytes in place; they stay ours
    plm_ = plm_create_with_memory(data_.data(), data_.size(), 0);
  }
  ~MpegDecoder() override{
    if (plm_)
      plm_destroy(plm_);
  }
  bool init(){
    TR_USE(CGE_VideoMpeg);
    if (!plm_ || !plm_has_headers(plm_) || plm_get_num_video_streams(plm_) == 0){
      TR_ERROR("No MPEG-1 video found");
      return false;
    }
    plm_set_loop(plm_, 0);
    audio_ = plm_get_num_audio_streams(plm_) > 0;
    plm_set_audio_enabled(plm_, audio_);
    return true;
  }
  Format getFormat() const override {return Format::Mpeg1;}
  int getWidth() const override {return plm_get_width(plm_);}
  int getHeight() const override {return plm_get_height(plm_);}
  double getFrameRate() const override {return plm_get_framerate(plm_);}
  bool hasAudio() const override {return audio_;}
  int getSampleRate() const override {return audio_ ? plm_get_samplerate(plm_) : 0;}
  //pl_mpeg's samples are always stereo (mono is doubled)
  int getChannels() const override {return audio_ ? 2 : 0;}
  bool decodeFrame(double* time) override{
    frame_ = plm_decode_video(plm_);
    if (!frame_)
      return false;
    *time = frame_->time;
    return true;
  }
  void convertFrame(unsigned char* rgba) override{
    if (!frame_)
      return;
    VideoInternal::yuvToRgba({frame_->y.data, (int)frame_->y.width}, {frame_->cb.data, (int)frame_->cb.width},
      {frame_->cr.data, (int)frame_->cr.width}, 1, 1, 0, 0, getWidth(), getHeight(), rgba);
  }
  bool decodeAudio(std::vector<float>& samples) override{
    if (!audio_)
      return false;
    plm_samples_t* decoded = plm_decode_audio(plm_);
    if (!decoded)
      return false;
    samples.insert(samples.end(), decoded->interleaved, decoded->interleaved + decoded->count*2);
    return true;
  }
  bool rewind() override{
    plm_rewind(plm_);
    frame_ = nullptr;
    return true;
  }
private:
  std::vector<unsigned char> data_;
  plm_t* plm_ = nullptr;
  plm_frame_t* frame_ = nullptr;
  bool audio_ = false;
};

}

std::unique_ptr<VideoDecoder> VideoInternal::openMpeg(std::vector<unsigned char>& data){
  auto decoder = std::make_unique<MpegDecoder>(data);
  if (!decoder->init())
    return nullptr;
  return decoder;
}
