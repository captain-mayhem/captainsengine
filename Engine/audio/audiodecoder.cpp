// first: it includes miniaudio, which includes windows.h without min and max
#include "audio_internal.h"
#include "audiodecoder.h"

using namespace CGE;

struct AudioDecoder::Impl{
  //! the bytes the decoder reads
  std::vector<uint8> file;
  ma_decoder decoder;
  bool decoderInit = false;
  ma_uint32 channels = 0;
  ma_uint32 sampleRate = 0;

  ~Impl(){
    if (decoderInit)
      ma_decoder_uninit(&decoder);
  }
};

std::unique_ptr<AudioDecoder> AudioDecoder::open(std::vector<uint8_t> file){
  if (file.empty())
    return nullptr;
  auto impl = std::make_unique<Impl>();
  impl->file = std::move(file);
  ma_decoder_config config = AudioInternal::decoderConfig();
  if (ma_decoder_init_memory(impl->file.data(), impl->file.size(), &config, &impl->decoder) != MA_SUCCESS)
    return nullptr;
  impl->decoderInit = true;
  ma_format format;
  if (ma_decoder_get_data_format(&impl->decoder, &format, &impl->channels, &impl->sampleRate, nullptr, 0) != MA_SUCCESS ||
    impl->channels == 0 || impl->sampleRate == 0)
    return nullptr;
  return std::unique_ptr<AudioDecoder>(new AudioDecoder(std::move(impl)));
}

AudioDecoder::AudioDecoder(std::unique_ptr<Impl> impl) : impl_(std::move(impl)){
}

AudioDecoder::~AudioDecoder(){
}

int AudioDecoder::getSampleRate() const{
  return (int)impl_->sampleRate;
}

int AudioDecoder::getChannels() const{
  return (int)impl_->channels;
}

unsigned long long AudioDecoder::getLength(){
  ma_uint64 length = 0;
  if (ma_decoder_get_length_in_pcm_frames(&impl_->decoder, &length) != MA_SUCCESS)
    return 0;
  return length;
}

size_t AudioDecoder::read(float* samples, size_t frames){
  //a decoder may give less than asked before its end (Ogg Vorbis at a page), so it is asked again
  size_t done = 0;
  while (done < frames){
    ma_uint64 got = 0;
    ma_result result = ma_decoder_read_pcm_frames(&impl_->decoder, samples+done*impl_->channels, frames-done, &got);
    done += (size_t)got;
    if (result != MA_SUCCESS || got == 0)
      break;
  }
  return done;
}

bool AudioDecoder::seek(unsigned long long frame){
  return ma_decoder_seek_to_pcm_frame(&impl_->decoder, frame) == MA_SUCCESS;
}
