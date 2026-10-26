// Theora video and Vorbis sound in Ogg, decoded with libtheora and libvorbis.
// The demuxing follows libtheora's player example: the pages go into one
// stream state per codec, which hands out its packets when asked, so the
// pictures and the sound can be decoded at their own pace.

#include "video_internal.h"

#include <algorithm>
#include <cstring>

#include <theora/theoradec.h>
#include <vorbis/codec.h>

#include <io/Tracing.h>

using namespace CGE;

TR_CHANNEL(CGE_VideoTheora);

namespace{

class TheoraDecoder : public VideoDecoder{
public:
  explicit TheoraDecoder(std::vector<unsigned char>& data) : data_(std::move(data)){
  }
  ~TheoraDecoder() override{
    clear();
  }
  bool init();
  Format getFormat() const override {return Format::Theora;}
  int getWidth() const override {return (int)info_.pic_width;}
  int getHeight() const override {return (int)info_.pic_height;}
  double getFrameRate() const override {return info_.fps_denominator == 0 ? 0 : (double)info_.fps_numerator/info_.fps_denominator;}
  bool hasAudio() const override {return vorbis_;}
  int getSampleRate() const override {return vorbis_ ? (int)vorbisInfo_.rate : 0;}
  int getChannels() const override {return vorbis_ ? vorbisInfo_.channels : 0;}
  bool decodeFrame(double* time) override;
  void convertFrame(unsigned char* rgba) override;
  bool decodeAudio(std::vector<float>& samples) override;
  bool rewind() override{
    clear();
    return init();
  }
private:
  //! gives the next chunk of the file to the Ogg sync; false at its end
  bool readData();
  //! reads the next page into the streams; false at the end
  bool readPage();
  //! the next packet of a stream; false at the end
  bool nextPacket(ogg_stream_state& stream, ogg_packet& packet);
  void clear();

  std::vector<unsigned char> data_;
  size_t pos_ = 0;
  ogg_sync_state sync_;
  bool syncInit_ = false;
  ogg_stream_state theoraStream_;
  bool theora_ = false;
  th_info info_;
  th_comment comment_;
  th_dec_ctx* decoder_ = nullptr;
  bool decoded_ = false;
  ogg_stream_state vorbisStream_;
  bool vorbis_ = false;
  vorbis_info vorbisInfo_;
  vorbis_comment vorbisComment_;
  vorbis_dsp_state vorbisDsp_;
  vorbis_block vorbisBlock_;
  bool vorbisInit_ = false;
};

bool TheoraDecoder::readData(){
  if (pos_ >= data_.size())
    return false;
  long chunk = (long)std::min<size_t>(data_.size()-pos_, 65536);
  char* buffer = ogg_sync_buffer(&sync_, chunk);
  memcpy(buffer, data_.data()+pos_, chunk);
  ogg_sync_wrote(&sync_, chunk);
  pos_ += chunk;
  return true;
}

bool TheoraDecoder::readPage(){
  ogg_page page;
  while (ogg_sync_pageout(&sync_, &page) <= 0){
    if (!readData())
      return false;
  }
  //each stream takes only its own pages
  if (theora_)
    ogg_stream_pagein(&theoraStream_, &page);
  if (vorbis_)
    ogg_stream_pagein(&vorbisStream_, &page);
  return true;
}

bool TheoraDecoder::nextPacket(ogg_stream_state& stream, ogg_packet& packet){
  for (;;){
    int result = ogg_stream_packetout(&stream, &packet);
    if (result > 0)
      return true;
    //a gap (result < 0) is skipped
    if (result == 0 && !readPage())
      return false;
  }
}

bool TheoraDecoder::init(){
  TR_USE(CGE_VideoTheora);
  pos_ = 0;
  ogg_sync_init(&sync_);
  syncInit_ = true;
  th_info_init(&info_);
  th_comment_init(&comment_);
  vorbis_info_init(&vorbisInfo_);
  vorbis_comment_init(&vorbisComment_);
  th_setup_info* setup = nullptr;
  //the first pages of the streams (beginning of stream) tell their codecs
  int theoraHeaders = 0;
  int vorbisHeaders = 0;
  bool started = false;
  while (!started && readData()){
    ogg_page page;
    while (ogg_sync_pageout(&sync_, &page) > 0){
      if (!ogg_page_bos(&page)){
        //the first data page
        if (theora_)
          ogg_stream_pagein(&theoraStream_, &page);
        if (vorbis_)
          ogg_stream_pagein(&vorbisStream_, &page);
        started = true;
        break;
      }
      ogg_stream_state test;
      ogg_stream_init(&test, ogg_page_serialno(&page));
      ogg_stream_pagein(&test, &page);
      ogg_packet packet;
      if (ogg_stream_packetout(&test, &packet) == 1){
        if (!theora_ && th_decode_headerin(&info_, &comment_, &setup, &packet) >= 0){
          theoraStream_ = test;
          theora_ = true;
          theoraHeaders = 1;
          continue;
        }
        if (!vorbis_ && vorbis_synthesis_headerin(&vorbisInfo_, &vorbisComment_, &packet) >= 0){
          vorbisStream_ = test;
          vorbis_ = true;
          vorbisHeaders = 1;
          continue;
        }
      }
      //another kind of stream (e.g. Skeleton), not decoded
      ogg_stream_clear(&test);
    }
  }
  if (!theora_){
    TR_ERROR("No Theora video found");
    th_setup_free(setup);
    return false;
  }
  //the remaining headers: Theora has three, Vorbis three
  bool theoraDone = false;
  while (!theoraDone || (vorbis_ && vorbisHeaders < 3)){
    ogg_packet packet;
    int result;
    while (!theoraDone && (result = ogg_stream_packetpeek(&theoraStream_, &packet)) != 0){
      if (result < 0)
        continue;
      int header = th_decode_headerin(&info_, &comment_, &setup, &packet);
      if (header < 0){
        TR_ERROR("Broken Theora header");
        th_setup_free(setup);
        return false;
      }
      //0: the first picture, which stays queued
      if (header == 0){
        theoraDone = true;
        break;
      }
      ogg_stream_packetout(&theoraStream_, nullptr);
      ++theoraHeaders;
    }
    while (vorbis_ && vorbisHeaders < 3 && (result = ogg_stream_packetout(&vorbisStream_, &packet)) != 0){
      if (result < 0 || vorbis_synthesis_headerin(&vorbisInfo_, &vorbisComment_, &packet) != 0){
        //the pictures can do without the sound
        TR_WARN("Broken Vorbis header: the video plays without sound");
        ogg_stream_clear(&vorbisStream_);
        vorbis_ = false;
        break;
      }
      ++vorbisHeaders;
    }
    if (theoraDone && (!vorbis_ || vorbisHeaders >= 3))
      break;
    if (!readPage()){
      //a video without pictures ends in its headers
      if (theoraHeaders < 3 || !setup){
        TR_ERROR("The Theora headers are incomplete");
        th_setup_free(setup);
        return false;
      }
      break;
    }
  }
  decoder_ = th_decode_alloc(&info_, setup);
  th_setup_free(setup);
  if (!decoder_){
    TR_ERROR("The Theora decoder can't be created");
    return false;
  }
  if (vorbis_){
    vorbis_synthesis_init(&vorbisDsp_, &vorbisInfo_);
    vorbis_block_init(&vorbisDsp_, &vorbisBlock_);
    vorbisInit_ = true;
  }
  return true;
}

void TheoraDecoder::clear(){
  if (decoder_)
    th_decode_free(decoder_);
  decoder_ = nullptr;
  decoded_ = false;
  if (theora_)
    ogg_stream_clear(&theoraStream_);
  theora_ = false;
  th_comment_clear(&comment_);
  th_info_clear(&info_);
  if (vorbisInit_){
    vorbis_block_clear(&vorbisBlock_);
    vorbis_dsp_clear(&vorbisDsp_);
  }
  vorbisInit_ = false;
  if (vorbis_)
    ogg_stream_clear(&vorbisStream_);
  vorbis_ = false;
  vorbis_comment_clear(&vorbisComment_);
  vorbis_info_clear(&vorbisInfo_);
  if (syncInit_)
    ogg_sync_clear(&sync_);
  syncInit_ = false;
}

bool TheoraDecoder::decodeFrame(double* time){
  ogg_packet packet;
  while (nextPacket(theoraStream_, packet)){
    ogg_int64_t granule = -1;
    int result = th_decode_packetin(decoder_, &packet, &granule);
    //a duplicate is the last picture again
    if (result == 0 || result == TH_DUPFRAME){
      double rate = getFrameRate();
      *time = rate > 0 ? th_granule_frame(decoder_, granule)/rate : 0;
      decoded_ = true;
      return true;
    }
  }
  return false;
}

void TheoraDecoder::convertFrame(unsigned char* rgba){
  if (!decoded_)
    return;
  th_ycbcr_buffer buffer;
  if (th_decode_ycbcr_out(decoder_, buffer) != 0)
    return;
  //4:2:0 halves the chroma both ways, 4:2:2 only across, 4:4:4 not
  int xShift = info_.pixel_fmt == TH_PF_444 ? 0 : 1;
  int yShift = info_.pixel_fmt == TH_PF_420 ? 1 : 0;
  VideoInternal::yuvToRgba({buffer[0].data, buffer[0].stride}, {buffer[1].data, buffer[1].stride},
    {buffer[2].data, buffer[2].stride}, xShift, yShift, (int)info_.pic_x, (int)info_.pic_y,
    getWidth(), getHeight(), rgba);
}

bool TheoraDecoder::decodeAudio(std::vector<float>& samples){
  if (!vorbis_)
    return false;
  for (;;){
    float** pcm;
    int frames = vorbis_synthesis_pcmout(&vorbisDsp_, &pcm);
    if (frames > 0){
      int channels = vorbisInfo_.channels;
      size_t start = samples.size();
      samples.resize(start + (size_t)frames*channels);
      float* out = &samples[start];
      for (int i = 0; i < frames; ++i){
        for (int c = 0; c < channels; ++c)
          *out++ = pcm[c][i];
      }
      vorbis_synthesis_read(&vorbisDsp_, frames);
      return true;
    }
    ogg_packet packet;
    if (!nextPacket(vorbisStream_, packet))
      return false;
    if (vorbis_synthesis(&vorbisBlock_, &packet) == 0)
      vorbis_synthesis_blockin(&vorbisDsp_, &vorbisBlock_);
  }
}

}

std::unique_ptr<VideoDecoder> VideoInternal::openTheora(std::vector<unsigned char>& data){
  auto decoder = std::make_unique<TheoraDecoder>(data);
  if (!decoder->init())
    return nullptr;
  return decoder;
}
