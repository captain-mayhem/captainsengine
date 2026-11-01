// Decoding for the audio module: miniaudio's own decoders (WAV, FLAC, MP3)
// and a decoding backend for Ogg Vorbis on libvorbis' vorbisfile.

#include "audio_internal.h"

#include <cstring>

#include <vorbis/vorbisfile.h>

using namespace CGE;

namespace{

//! an Ogg Vorbis stream as a miniaudio data source; ma_data_source_base must come first
struct VorbisSource{
  ma_data_source_base base;
  //! how the decoder reads the file
  ma_read_proc onRead;
  ma_seek_proc onSeek;
  ma_tell_proc onTell;
  void* io;
  OggVorbis_File file;
  //! the first part's; a chained stream's later parts are converted to them
  ma_uint32 channels;
  ma_uint32 sampleRate;
  //! a later part at another rate goes through the resampler (set up when one comes)
  ma_linear_resampler resampler;
  bool resampling;
  //! the part the resampler is set up for, -1 for none
  int resampledSection;
  //! that part's frames read but not resampled yet (PENDING frames of channels)
  float* pending;
  ma_uint64 pendingFrames;
  ma_uint64 pendingPos;
};

//! the frames read from vorbisfile at a time
const int PENDING = 4096;

size_t vorbisRead(void* ptr, size_t size, size_t count, void* source){
  VorbisSource* src = (VorbisSource*)source;
  size_t read = 0;
  if (size == 0 || (src->onRead(src->io, ptr, size*count, &read) != MA_SUCCESS && read == 0))
    return 0;
  return read/size;
}

int vorbisSeek(void* source, ogg_int64_t offset, int whence){
  VorbisSource* src = (VorbisSource*)source;
  ma_seek_origin origin = whence == SEEK_SET ? ma_seek_origin_start : whence == SEEK_END ? ma_seek_origin_end : ma_seek_origin_current;
  return src->onSeek(src->io, offset, origin) == MA_SUCCESS ? 0 : -1;
}

long vorbisTell(void* source){
  VorbisSource* src = (VorbisSource*)source;
  ma_int64 pos = 0;
  if (src->onTell(src->io, &pos) != MA_SUCCESS)
    return -1;
  return (long)pos;
}

//! prepares the resampler for a part at rate; false if it can't be set up
bool vorbisResampleFrom(VorbisSource* src, int section, ma_uint32 rate){
  if (!src->resampling){
    ma_linear_resampler_config config = ma_linear_resampler_config_init(ma_format_f32, src->channels, rate, src->sampleRate);
    if (ma_linear_resampler_init(&config, nullptr, &src->resampler) != MA_SUCCESS)
      return false;
    src->pending = (float*)ma_malloc(sizeof(float)*PENDING*src->channels, nullptr);
    if (!src->pending){
      ma_linear_resampler_uninit(&src->resampler, nullptr);
      return false;
    }
    src->resampling = true;
  }
  else if (section != src->resampledSection){
    //a new part starts without the last one's history
    ma_linear_resampler_set_rate(&src->resampler, rate, src->sampleRate);
    ma_linear_resampler_reset(&src->resampler);
  }
  src->resampledSection = section;
  return true;
}

ma_result vorbisReadFrames(ma_data_source* source, void* out, ma_uint64 frameCount, ma_uint64* framesRead){
  VorbisSource* src = (VorbisSource*)source;
  float* dst = (float*)out;
  ma_uint64 done = 0;
  while (done < frameCount){
    //first what's left of a part at another rate
    if (src->pendingPos < src->pendingFrames){
      ma_uint64 in = src->pendingFrames-src->pendingPos;
      ma_uint64 made = frameCount-done;
      ma_linear_resampler_process_pcm_frames(&src->resampler, src->pending+src->pendingPos*src->channels, &in, dst, &made);
      src->pendingPos += in;
      dst += made*src->channels;
      done += made;
      if (in == 0 && made == 0)
        break;
      continue;
    }
    float** pcm;
    int section;
    int want = (int)(frameCount-done < PENDING ? frameCount-done : PENDING);
    long got = ov_read_float(&src->file, &pcm, want, &section);
    if (got == OV_HOLE)
      continue;
    if (got <= 0)
      break;
    //a chained stream's later part may have other channels; it is mapped onto the first part's
    vorbis_info* info = ov_info(&src->file, section);
    ma_uint32 channels = info ? (ma_uint32)info->channels : src->channels;
    //and another rate; it is resampled to the first part's (it played too fast or too slow before)
    ma_uint32 rate = info ? (ma_uint32)info->rate : src->sampleRate;
    bool resample = rate != src->sampleRate;
    if (resample && !vorbisResampleFrom(src, section, rate))
      break;
    float* to = resample ? src->pending : dst;
    for (long i = 0; i < got; ++i){
      for (ma_uint32 c = 0; c < src->channels; ++c)
        to[c] = pcm[c < channels ? c : channels-1][i];
      to += src->channels;
    }
    if (resample){
      src->pendingFrames = (ma_uint64)got;
      src->pendingPos = 0;
    }
    else{
      dst = to;
      done += got;
    }
  }
  if (framesRead)
    *framesRead = done;
  //the end only when nothing came; miniaudio reads again for the rest
  return done == 0 && frameCount > 0 ? MA_AT_END : MA_SUCCESS;
}

ma_result vorbisSeekFrames(ma_data_source* source, ma_uint64 frame){
  VorbisSource* src = (VorbisSource*)source;
  //the frame is counted in the file's own rates (a resampled part's frames are its own)
  src->pendingFrames = src->pendingPos = 0;
  src->resampledSection = -1;
  return ov_pcm_seek(&src->file, (ogg_int64_t)frame) == 0 ? MA_SUCCESS : MA_BAD_SEEK;
}

ma_result vorbisFormat(ma_data_source* source, ma_format* format, ma_uint32* channels, ma_uint32* sampleRate, ma_channel* channelMap, size_t channelMapCap){
  VorbisSource* src = (VorbisSource*)source;
  if (format)
    *format = ma_format_f32;
  if (channels)
    *channels = src->channels;
  if (sampleRate)
    *sampleRate = src->sampleRate;
  if (channelMap)
    ma_channel_map_init_standard(ma_standard_channel_map_vorbis, channelMap, channelMapCap, src->channels);
  return MA_SUCCESS;
}

ma_result vorbisCursor(ma_data_source* source, ma_uint64* cursor){
  VorbisSource* src = (VorbisSource*)source;
  ogg_int64_t pos = ov_pcm_tell(&src->file);
  *cursor = pos < 0 ? 0 : (ma_uint64)pos;
  return pos < 0 ? MA_ERROR : MA_SUCCESS;
}

ma_result vorbisLength(ma_data_source* source, ma_uint64* length){
  VorbisSource* src = (VorbisSource*)source;
  ogg_int64_t total = ov_seekable(&src->file) ? 0 : -1;
  //the parts at the first part's rate, as they are read
  for (int i = 0; total >= 0 && i < ov_streams(&src->file); ++i){
    ogg_int64_t frames = ov_pcm_total(&src->file, i);
    vorbis_info* info = ov_info(&src->file, i);
    if (frames < 0 || !info || info->rate <= 0)
      total = -1;
    else
      total += (ogg_int64_t)info->rate == (ogg_int64_t)src->sampleRate ? frames : frames*(ogg_int64_t)src->sampleRate/info->rate;
  }
  *length = total < 0 ? 0 : (ma_uint64)total;
  return total < 0 ? MA_NOT_IMPLEMENTED : MA_SUCCESS;
}

ma_data_source_vtable vorbisSourceVtable = {
  vorbisReadFrames,
  vorbisSeekFrames,
  vorbisFormat,
  vorbisCursor,
  vorbisLength,
  nullptr,
  0
};

ma_result vorbisInit(void* userData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* io,
  const ma_decoding_backend_config* config, const ma_allocation_callbacks* alloc, ma_data_source** backend){
  VorbisSource* src = (VorbisSource*)ma_malloc(sizeof(VorbisSource), alloc);
  if (!src)
    return MA_OUT_OF_MEMORY;
  memset(src, 0, sizeof(VorbisSource));
  src->onRead = onRead;
  src->onSeek = onSeek;
  src->onTell = onTell;
  src->io = io;
  ma_data_source_config dsConfig = ma_data_source_config_init();
  dsConfig.vtable = &vorbisSourceVtable;
  if (ma_data_source_init(&dsConfig, &src->base) != MA_SUCCESS){
    ma_free(src, alloc);
    return MA_ERROR;
  }
  ov_callbacks callbacks = {vorbisRead, vorbisSeek, nullptr, vorbisTell};
  if (ov_open_callbacks(src, &src->file, nullptr, 0, callbacks) != 0){
    ma_data_source_uninit(&src->base);
    ma_free(src, alloc);
    return MA_INVALID_FILE;
  }
  vorbis_info* info = ov_info(&src->file, -1);
  if (!info || info->channels <= 0 || info->channels > MA_MAX_CHANNELS){
    ov_clear(&src->file);
    ma_data_source_uninit(&src->base);
    ma_free(src, alloc);
    return MA_INVALID_FILE;
  }
  src->channels = (ma_uint32)info->channels;
  src->sampleRate = (ma_uint32)info->rate;
  src->resampledSection = -1;
  *backend = src;
  return MA_SUCCESS;
}

void vorbisUninit(void* userData, ma_data_source* backend, const ma_allocation_callbacks* alloc){
  VorbisSource* src = (VorbisSource*)backend;
  if (src->resampling){
    ma_linear_resampler_uninit(&src->resampler, nullptr);
    ma_free(src->pending, nullptr);
  }
  ov_clear(&src->file);
  ma_data_source_uninit(&src->base);
  ma_free(src, alloc);
}

ma_decoding_backend_vtable vorbisBackend = {
  vorbisInit,
  nullptr,
  nullptr,
  nullptr,
  vorbisUninit
};

ma_decoding_backend_vtable* customBackends[] = {&vorbisBackend};

}

ma_decoder_config AudioInternal::decoderConfig(){
  ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
  config.ppCustomBackendVTables = customBackends;
  config.customBackendCount = 1;
  return config;
}

bool AudioInternal::decodeAll(const void* data, size_t size, DecodedSound& sound){
  ma_decoder_config config = decoderConfig();
  ma_decoder decoder;
  if (ma_decoder_init_memory(data, size, &config, &decoder) != MA_SUCCESS)
    return false;
  ma_format format;
  if (ma_decoder_get_data_format(&decoder, &format, &sound.channels, &sound.sampleRate, nullptr, 0) != MA_SUCCESS ||
    sound.channels == 0 || sound.sampleRate == 0){
    ma_decoder_uninit(&decoder);
    return false;
  }
  //the length isn't known for every file (an MP3 without a header tells it only at the end)
  ma_uint64 length = 0;
  if (ma_decoder_get_length_in_pcm_frames(&decoder, &length) == MA_SUCCESS && length > 0)
    sound.frames.reserve((size_t)(length*sound.channels));
  const ma_uint64 chunk = 4096;
  std::vector<float> buffer((size_t)(chunk*sound.channels));
  sound.frameCount = 0;
  for (;;){
    ma_uint64 read = 0;
    ma_result result = ma_decoder_read_pcm_frames(&decoder, buffer.data(), chunk, &read);
    sound.frames.insert(sound.frames.end(), buffer.begin(), buffer.begin()+(size_t)(read*sound.channels));
    sound.frameCount += read;
    if (result != MA_SUCCESS || read < chunk)
      break;
  }
  ma_decoder_uninit(&decoder);
  return sound.frameCount > 0;
}
