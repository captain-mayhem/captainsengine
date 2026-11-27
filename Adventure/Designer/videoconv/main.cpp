// videoconv: converts a game's video into a format the Adventure plays (Ogg
// Theora with Vorbis sound), for the StoryDesigner, when a video is added.
//
//   videoconv --check <file>            exit code 0: plays as it is (MPEG-1 with
//                                       MP2, or Ogg Theora with Vorbis), 1: needs
//                                       converting, 2: can't be read
//   videoconv [-q quality] <in> <out>   converts; quality 0-10 (default 7)
//
// The input is decoded with Windows' codecs, as PCDK played its videos:
// Media Foundation first (MPEG-1/2, WMV, MPEG-4 part 2, H.264, MJPEG, ...),
// then Video for Windows for the AVI codecs that only it has (Cinepak,
// Microsoft Video 1, RLE, or e.g. an installed Indeo). The output has a
// constant frame rate; its lines "progress <percent>" go to stdout.

#define NOMINMAX
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <vfw.h>
#include <msacm.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include <theora/theoraenc.h>
#include <vorbis/vorbisenc.h>

#define PL_MPEG_IMPLEMENTATION
#include <pl_mpeg.h>

namespace{

//! what a source delivers: pictures (RGB, rows from the top) and sound (interleaved float)
class Source{
public:
  virtual ~Source() = default;
  int width = 0;
  int height = 0;
  //! frames per second as a fraction
  unsigned fpsNum = 25;
  unsigned fpsDen = 1;
  int sampleRate = 0;
  int channels = 0;
  //! the length in seconds, 0 if unknown
  double duration = 0;
  //! the next picture (time in seconds) or sound; false at the end of both
  enum class Kind{Picture, Sound, End};
  virtual Kind read(double* time, std::vector<unsigned char>& rgb, std::vector<float>& sound) = 0;
};

template<class T> void release(T*& p){
  if (p)
    p->Release();
  p = nullptr;
}

//! decodes with Media Foundation's source reader
class MediaFoundationSource : public Source{
public:
  ~MediaFoundationSource() override{
    release(reader_);
  }
  bool open(const std::wstring& path){
    IMFAttributes* attributes = nullptr;
    MFCreateAttributes(&attributes, 1);
    //colour conversion to RGB32 where a decoder has no NV12 (see setVideo); not
    //the advanced processor, whose RGB was clearly worse (Xvid: 29 dB against 40)
    attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    HRESULT hr = MFCreateSourceReaderFromURL(path.c_str(), attributes, &reader_);
    release(attributes);
    if (FAILED(hr)){
      fprintf(stderr, "Media Foundation can't open it (0x%08lx)\n", (unsigned long)hr);
      return false;
    }
    reader_->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
    if (!setVideo())
      return false;
    setAudio();
    PROPVARIANT var;
    PropVariantInit(&var);
    if (SUCCEEDED(reader_->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var)))
      duration = var.uhVal.QuadPart/1e7;
    PropVariantClear(&var);
    return true;
  }
  Kind read(double* time, std::vector<unsigned char>& rgb, std::vector<float>& sound) override{
    for (;;){
      if (videoEnded_ && (audioEnded_ || channels == 0))
        return Kind::End;
      DWORD stream = 0, flags = 0;
      LONGLONG timestamp = 0;
      IMFSample* sample = nullptr;
      HRESULT hr = reader_->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM, 0, &stream, &flags, &timestamp, &sample);
      if (FAILED(hr)){
        fprintf(stderr, "Decoding failed (0x%08lx)\n", (unsigned long)hr);
        return Kind::End;
      }
      if (flags & MF_SOURCE_READERF_ERROR){
        fprintf(stderr, "Decoding failed\n");
        return Kind::End;
      }
      bool isVideo = stream == videoStream_;
      if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
        (isVideo ? videoEnded_ : audioEnded_) = true;
      if (isVideo && (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED))
        readVideoType();
      if (!sample)
        continue;
      //a picture's own buffer (a copy made contiguous has no 2D interface)
      DWORD buffers = 0;
      sample->GetBufferCount(&buffers);
      IMFMediaBuffer* buffer = nullptr;
      if (isVideo && buffers == 1)
        sample->GetBufferByIndex(0, &buffer);
      else
        sample->ConvertToContiguousBuffer(&buffer);
      BYTE* data = nullptr;
      DWORD length = 0;
      buffer->GetCurrentLength(&length);
      *time = timestamp/1e7;
      Kind kind;
      //a picture's top row, pitch (negative bottom-up) and length come from its 2D
      //buffer: its rows may be padded, e.g. to 16 for odd sizes; else from the media type
      IMF2DBuffer2* buffer2d = nullptr;
      BYTE* top = nullptr;
      LONG pitch = 0;
      bool locked2d = false;
      if (isVideo){
        BYTE* start = nullptr;
        locked2d = SUCCEEDED(buffer->QueryInterface(IID_PPV_ARGS(&buffer2d))) &&
          SUCCEEDED(buffer2d->Lock2DSize(MF2DBuffer_LockFlags_Read, &top, &pitch, &start, &length));
        if (!locked2d){
          buffer->Lock(&data, nullptr, &length);
          pitch = stride_ != 0 ? stride_ : width*(nv12_ ? 1 : 4);
          top = pitch >= 0 ? data : data + (size_t)(-pitch)*(height-1);
        }
      }
      else
        buffer->Lock(&data, nullptr, &length);
      if (isVideo && nv12_){
        //the luma rows, then the interleaved Cb Cr rows, both with the pitch (the rows' padding)
        DWORD rows = pitch > 0 ? length*2/3/pitch : 0;
        const BYTE* chroma = top + (size_t)pitch*rows;
        rgb.resize((size_t)width*height*3);
        for (int y = 0; y < height && (DWORD)y < rows; ++y){
          unsigned char* dst = &rgb[(size_t)y*width*3];
          for (int x = 0; x < width; ++x){
            int c = 298*(top[(size_t)y*pitch + x]-16) + 128;
            const BYTE* uv = chroma + (size_t)(y/2)*pitch + (x & ~1);
            int d = uv[0]-128;
            int e = uv[1]-128;
            dst[0] = (unsigned char)std::clamp((c + 409*e) >> 8, 0, 255);
            dst[1] = (unsigned char)std::clamp((c - 100*d - 208*e) >> 8, 0, 255);
            dst[2] = (unsigned char)std::clamp((c + 516*d) >> 8, 0, 255);
            dst += 3;
          }
        }
        kind = Kind::Picture;
      }
      else if (isVideo){
        //RGB32, BGRA in memory
        rgb.resize((size_t)width*height*3);
        for (int y = 0; y < height; ++y){
          const BYTE* src = top + (ptrdiff_t)y*pitch;
          unsigned char* dst = &rgb[(size_t)y*width*3];
          for (int x = 0; x < width; ++x){
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            src += 4;
            dst += 3;
          }
        }
        kind = Kind::Picture;
      }
      else{
        const float* samples = (const float*)data;
        sound.assign(samples, samples + length/sizeof(float));
        kind = Kind::Sound;
      }
      if (locked2d)
        buffer2d->Unlock2D();
      else
        buffer->Unlock();
      release(buffer2d);
      release(buffer);
      release(sample);
      return kind;
    }
  }
private:
  bool setVideo(){
    DWORD index = (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    IMFMediaType* native = nullptr;
    if (FAILED(reader_->GetNativeMediaType(index, 0, &native))){
      fprintf(stderr, "Media Foundation finds no video in it\n");
      return false;
    }
    UINT32 num = 0, den = 0;
    if (SUCCEEDED(MFGetAttributeRatio(native, MF_MT_FRAME_RATE, &num, &den)) && num > 0 && den > 0){
      fpsNum = num;
      fpsDen = den;
    }
    GUID subtype = GUID_NULL;
    native->GetGUID(MF_MT_SUBTYPE, &subtype);
    release(native);
    IMFMediaType* type = nullptr;
    MFCreateMediaType(&type);
    type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    //the decoders' own NV12 first, converted here (one conversion fewer: WMV 35 dB
    //against 33 through RGB32), else RGB32 (e.g. uncompressed RGB, DivX 3)
    type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    HRESULT hr = reader_->SetCurrentMediaType(index, nullptr, type);
    nv12_ = SUCCEEDED(hr);
    if (FAILED(hr)){
      type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
      hr = reader_->SetCurrentMediaType(index, nullptr, type);
    }
    release(type);
    if (FAILED(hr)){
      if (subtype == MFVideoFormat_MPEG2 || subtype == MFVideoFormat_MPG1)
        fprintf(stderr, "Windows has no MPEG-1/2 video decoder: install Microsoft's \"MPEG-2 Video Extension\" (Microsoft Store)\n");
      else
        fprintf(stderr, "Media Foundation has no decoder for the video (0x%08lx)\n", (unsigned long)hr);
      return false;
    }
    reader_->SetStreamSelection(index, TRUE);
    videoStream_ = streamIndex(index);
    return readVideoType();
  }
  bool readVideoType(){
    IMFMediaType* type = nullptr;
    if (FAILED(reader_->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type)))
      return false;
    UINT32 w = 0, h = 0;
    MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &w, &h);
    width = (int)w;
    height = (int)h;
    UINT32 stride = 0;
    if (SUCCEEDED(type->GetUINT32(MF_MT_DEFAULT_STRIDE, &stride)))
      stride_ = (LONG)stride;
    else
      MFGetStrideForBitmapInfoHeader((nv12_ ? MFVideoFormat_NV12 : MFVideoFormat_RGB32).Data1, w, &stride_);
    release(type);
    return width > 0 && height > 0;
  }
  void setAudio(){
    DWORD index = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;
    IMFMediaType* native = nullptr;
    if (FAILED(reader_->GetNativeMediaType(index, 0, &native)))
      return;
    UINT32 nativeChannels = MFGetAttributeUINT32(native, MF_MT_AUDIO_NUM_CHANNELS, 2);
    UINT32 nativeRate = MFGetAttributeUINT32(native, MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
    release(native);
    IMFMediaType* type = nullptr;
    MFCreateMediaType(&type);
    type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
    //mono stays mono, more channels become stereo
    type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, std::min<UINT32>(nativeChannels, 2));
    type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, nativeRate);
    HRESULT hr = reader_->SetCurrentMediaType(index, nullptr, type);
    release(type);
    if (FAILED(hr)){
      fprintf(stderr, "The sound can't be decoded: the video is converted without it\n");
      return;
    }
    reader_->SetStreamSelection(index, TRUE);
    audioStream_ = streamIndex(index);
    IMFMediaType* current = nullptr;
    reader_->GetCurrentMediaType(index, &current);
    channels = (int)MFGetAttributeUINT32(current, MF_MT_AUDIO_NUM_CHANNELS, 0);
    sampleRate = (int)MFGetAttributeUINT32(current, MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
    release(current);
  }
  //! the actual index of the first stream of a kind, as ReadSample reports it
  DWORD streamIndex(DWORD first){
    for (DWORD i = 0; ; ++i){
      IMFMediaType* type = nullptr;
      if (FAILED(reader_->GetNativeMediaType(i, 0, &type)))
        return first;
      GUID major;
      type->GetGUID(MF_MT_MAJOR_TYPE, &major);
      release(type);
      if (major == (first == (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM ? MFMediaType_Video : MFMediaType_Audio))
        return i;
    }
  }
  IMFSourceReader* reader_ = nullptr;
  DWORD videoStream_ = 0;
  DWORD audioStream_ = (DWORD)-1;
  LONG stride_ = 0;
  bool nv12_ = false;
  bool videoEnded_ = false;
  bool audioEnded_ = false;
};

//! decodes an AVI with Video for Windows (the VCM and ACM codecs)
class VfwSource : public Source{
public:
  ~VfwSource() override{
    if (acm_)
      acmStreamClose(acm_, 0);
    if (frames_)
      AVIStreamGetFrameClose(frames_);
    if (audio_)
      AVIStreamRelease(audio_);
    if (video_)
      AVIStreamRelease(video_);
    if (file_)
      AVIFileRelease(file_);
    AVIFileExit();
  }
  bool open(const std::wstring& path){
    AVIFileInit();
    HRESULT hr = AVIFileOpenW(&file_, path.c_str(), OF_READ | OF_SHARE_DENY_WRITE, nullptr);
    if (hr != 0){
      fprintf(stderr, "Video for Windows can't open it (0x%08lx)\n", (unsigned long)hr);
      return false;
    }
    if (AVIFileGetStream(file_, &video_, streamtypeVIDEO, 0) != 0)
      return false;
    AVISTREAMINFOW info;
    AVIStreamInfoW(video_, &info, sizeof(info));
    width = info.rcFrame.right - info.rcFrame.left;
    height = info.rcFrame.bottom - info.rcFrame.top;
    if (info.dwRate > 0 && info.dwScale > 0){
      fpsNum = info.dwRate;
      fpsDen = info.dwScale;
    }
    firstFrame_ = AVIStreamStart(video_);
    frameCount_ = AVIStreamLength(video_);
    duration = (double)frameCount_*fpsDen/fpsNum;
    //24-bit RGB, which the codecs deliver or VCM converts to
    BITMAPINFOHEADER want = {};
    want.biSize = sizeof(want);
    want.biWidth = width;
    want.biHeight = height;
    want.biPlanes = 1;
    want.biBitCount = 24;
    want.biCompression = BI_RGB;
    frames_ = AVIStreamGetFrameOpen(video_, &want);
    if (!frames_)
      frames_ = AVIStreamGetFrameOpen(video_, nullptr);
    if (!frames_){
      fprintf(stderr, "Video for Windows has no decoder for the video (%.4s)\n", (const char*)&info.fccHandler);
      return false;
    }
    openAudio();
    return true;
  }
  Kind read(double* time, std::vector<unsigned char>& rgb, std::vector<float>& sound) override{
    double frameTime = (double)nextFrame_*fpsDen/fpsNum;
    double soundTime = sampleRate > 0 ? (double)soundDone_/sampleRate : 1e30;
    bool pictures = nextFrame_ < frameCount_;
    if (!pictures && soundTime >= 1e30)
      return Kind::End;
    if (channels > 0 && soundTime < frameTime && readSound(sound)){
      *time = soundTime;
      return Kind::Sound;
    }
    if (!pictures){
      if (channels > 0 && readSound(sound)){
        *time = soundTime;
        return Kind::Sound;
      }
      return Kind::End;
    }
    *time = frameTime;
    BITMAPINFOHEADER* dib = (BITMAPINFOHEADER*)AVIStreamGetFrame(frames_, firstFrame_ + nextFrame_);
    ++nextFrame_;
    if (!dib){
      //a frame the codec can't give is the previous one again
      return Kind::Picture;
    }
    convertDib(dib, rgb);
    return Kind::Picture;
  }
private:
  void convertDib(BITMAPINFOHEADER* dib, std::vector<unsigned char>& rgb){
    rgb.resize((size_t)width*height*3);
    const unsigned char* bits = (const unsigned char*)dib + dib->biSize + dib->biClrUsed*sizeof(RGBQUAD);
    int bpp = dib->biBitCount;
    if (bpp <= 8 && dib->biClrUsed == 0)
      bits += (1 << bpp)*sizeof(RGBQUAD);
    const RGBQUAD* palette = (const RGBQUAD*)((const unsigned char*)dib + dib->biSize);
    size_t stride = ((size_t)dib->biWidth*bpp + 31)/32*4;
    bool bottomUp = dib->biHeight > 0;
    for (int y = 0; y < height; ++y){
      const unsigned char* src = bits + (bottomUp ? (size_t)(height-1-y) : (size_t)y)*stride;
      unsigned char* dst = &rgb[(size_t)y*width*3];
      for (int x = 0; x < width; ++x){
        if (bpp == 24 || bpp == 32){
          const unsigned char* p = src + (size_t)x*(bpp/8);
          dst[0] = p[2]; dst[1] = p[1]; dst[2] = p[0];
        }
        else if (bpp == 16){
          unsigned v = src[2*x] | (src[2*x+1] << 8);
          dst[0] = (unsigned char)(((v >> 10) & 31)*255/31);
          dst[1] = (unsigned char)(((v >> 5) & 31)*255/31);
          dst[2] = (unsigned char)((v & 31)*255/31);
        }
        else if (bpp == 8){
          const RGBQUAD& c = palette[src[x]];
          dst[0] = c.rgbRed; dst[1] = c.rgbGreen; dst[2] = c.rgbBlue;
        }
        dst += 3;
      }
    }
  }
  void openAudio(){
    if (AVIFileGetStream(file_, &audio_, streamtypeAUDIO, 0) != 0){
      audio_ = nullptr;
      return;
    }
    LONG size = 0;
    AVIStreamReadFormat(audio_, AVIStreamStart(audio_), nullptr, &size);
    format_.resize(std::max<LONG>(size, sizeof(WAVEFORMATEX)));
    AVIStreamReadFormat(audio_, AVIStreamStart(audio_), format_.data(), &size);
    WAVEFORMATEX* in = (WAVEFORMATEX*)format_.data();
    pcm_ = {};
    if (in->wFormatTag == WAVE_FORMAT_PCM && (in->wBitsPerSample == 8 || in->wBitsPerSample == 16)){
      pcm_ = *in;
    }
    else{
      //compressed: ACM converts to 16-bit PCM
      pcm_.wFormatTag = WAVE_FORMAT_PCM;
      if (acmFormatSuggest(nullptr, in, &pcm_, sizeof(pcm_), ACM_FORMATSUGGESTF_WFORMATTAG) != 0 ||
        acmStreamOpen(&acm_, nullptr, in, &pcm_, nullptr, 0, 0, ACM_STREAMOPENF_NONREALTIME) != 0){
        fprintf(stderr, "The sound can't be decoded: the video is converted without it\n");
        acm_ = nullptr;
        return;
      }
    }
    sampleRate = (int)pcm_.nSamplesPerSec;
    channels = std::min<int>(pcm_.nChannels, 2);
    nextSample_ = AVIStreamStart(audio_);
    endSample_ = AVIStreamEnd(audio_);
  }
  //! about a tenth of a second of sound
  bool readSound(std::vector<float>& sound){
    std::vector<unsigned char> pcm;
    while (pcm.empty()){
      if (nextSample_ >= endSample_ && pending_.empty())
        return false;
      if (nextSample_ < endSample_){
        LONG bytes = 0, samples = 0;
        WAVEFORMATEX* in = (WAVEFORMATEX*)format_.data();
        LONG want = std::max<LONG>(1, (LONG)(in->nAvgBytesPerSec/10/std::max<WORD>(in->nBlockAlign, 1)));
        std::vector<unsigned char> raw((size_t)want*std::max<WORD>(in->nBlockAlign, 1) + 65536);
        if (AVIStreamRead(audio_, nextSample_, want, raw.data(), (LONG)raw.size(), &bytes, &samples) != 0 || samples == 0){
          nextSample_ = endSample_;
        }
        else{
          nextSample_ += samples;
          pending_.insert(pending_.end(), raw.begin(), raw.begin()+bytes);
        }
      }
      if (!acm_){
        pcm.swap(pending_);
        pending_.clear();
        continue;
      }
      //ACM converts whole blocks; the rest waits for the next read
      bool last = nextSample_ >= endSample_;
      DWORD outSize = 0;
      acmStreamSize(acm_, (DWORD)std::max<size_t>(pending_.size(), 1), &outSize, ACM_STREAMSIZEF_SOURCE);
      std::vector<unsigned char> out(outSize + 4096);
      ACMSTREAMHEADER header = {};
      header.cbStruct = sizeof(header);
      header.pbSrc = pending_.data();
      header.cbSrcLength = (DWORD)pending_.size();
      header.pbDst = out.data();
      header.cbDstLength = (DWORD)out.size();
      if (pending_.empty() || acmStreamPrepareHeader(acm_, &header, 0) != 0){
        pending_.clear();
        continue;
      }
      acmStreamConvert(acm_, &header, last ? ACM_STREAMCONVERTF_END : ACM_STREAMCONVERTF_BLOCKALIGN);
      acmStreamUnprepareHeader(acm_, &header, 0);
      pcm.assign(out.begin(), out.begin()+header.cbDstLengthUsed);
      pending_.erase(pending_.begin(), pending_.begin()+std::min<size_t>(header.cbSrcLengthUsed, pending_.size()));
      if (last && header.cbSrcLengthUsed == 0)
        pending_.clear();
    }
    //to float, at most two channels
    int bits = pcm_.wBitsPerSample;
    int inChannels = pcm_.nChannels;
    size_t frames = pcm.size()/(bits/8)/inChannels;
    sound.resize(frames*channels);
    for (size_t i = 0; i < frames; ++i){
      for (int c = 0; c < channels; ++c){
        size_t at = i*inChannels + c;
        sound[i*channels + c] = bits == 8 ? (pcm[at]-128)/128.0f : (short)(pcm[2*at] | (pcm[2*at+1] << 8))/32768.0f;
      }
    }
    soundDone_ += frames;
    return true;
  }
  PAVIFILE file_ = nullptr;
  PAVISTREAM video_ = nullptr;
  PAVISTREAM audio_ = nullptr;
  PGETFRAME frames_ = nullptr;
  LONG firstFrame_ = 0;
  LONG frameCount_ = 0;
  LONG nextFrame_ = 0;
  std::vector<unsigned char> format_;
  WAVEFORMATEX pcm_ = {};
  HACMSTREAM acm_ = nullptr;
  LONG nextSample_ = 0;
  LONG endSample_ = 0;
  std::vector<unsigned char> pending_;
  size_t soundDone_ = 0;
};

//! encodes Theora and Vorbis into Ogg, the pages of both in time order
class OgvWriter{
public:
  ~OgvWriter(){
    if (encoder_)
      th_encode_free(encoder_);
    th_info_clear(&info_);
    th_comment_clear(&comment_);
    ogg_stream_clear(&video_);
    if (sound_){
      ogg_stream_clear(&audio_);
      vorbis_block_clear(&block_);
      vorbis_dsp_clear(&dsp_);
      vorbis_comment_clear(&vorbisComment_);
      vorbis_info_clear(&vorbis_);
    }
  }
  bool open(const Source& src, int quality){
    width_ = src.width;
    height_ = src.height;
    th_info_init(&info_);
    info_.frame_width = (width_+15) & ~15;
    info_.frame_height = (height_+15) & ~15;
    info_.pic_width = width_;
    info_.pic_height = height_;
    info_.fps_numerator = src.fpsNum;
    info_.fps_denominator = src.fpsDen;
    info_.aspect_numerator = 1;
    info_.aspect_denominator = 1;
    info_.colorspace = TH_CS_UNSPECIFIED;
    info_.pixel_fmt = TH_PF_420;
    //Theora's quality is 0-63
    info_.quality = std::clamp(quality, 0, 10)*63/10;
    encoder_ = th_encode_alloc(&info_);
    if (!encoder_)
      return false;
    th_comment_init(&comment_);
    th_comment_add_tag(&comment_, (char*)"ENCODER", (char*)"videoconv (CGE StoryDesigner)");
    ogg_stream_init(&video_, 1);
    ogg_packet packet;
    ogg_page page;
    //the first header packet alone on the first page (beginning of stream)
    if (th_encode_flushheader(encoder_, &comment_, &packet) <= 0)
      return false;
    ogg_stream_packetin(&video_, &packet);
    while (ogg_stream_flush(&video_, &page))
      writeHeaderPage(page);
    sound_ = src.channels > 0 && src.sampleRate > 0;
    ogg_packet comments, codebooks;
    if (sound_){
      vorbis_info_init(&vorbis_);
      if (vorbis_encode_init_vbr(&vorbis_, src.channels, src.sampleRate, std::clamp(quality, 0, 10)/10.0f*0.8f) != 0){
        fprintf(stderr, "The sound's format can't be encoded: the video is converted without it\n");
        vorbis_info_clear(&vorbis_);
        sound_ = false;
      }
    }
    if (sound_){
      channels_ = src.channels;
      vorbis_comment_init(&vorbisComment_);
      vorbis_analysis_init(&dsp_, &vorbis_);
      vorbis_block_init(&dsp_, &block_);
      ogg_stream_init(&audio_, 2);
      ogg_packet first;
      vorbis_analysis_headerout(&dsp_, &vorbisComment_, &first, &comments, &codebooks);
      ogg_stream_packetin(&audio_, &first);
      while (ogg_stream_flush(&audio_, &page))
        writeHeaderPage(page);
    }
    //the other headers of both, before the data
    while (th_encode_flushheader(encoder_, &comment_, &packet) > 0)
      ogg_stream_packetin(&video_, &packet);
    while (ogg_stream_flush(&video_, &page))
      writeHeaderPage(page);
    if (sound_){
      ogg_stream_packetin(&audio_, &comments);
      ogg_stream_packetin(&audio_, &codebooks);
      while (ogg_stream_flush(&audio_, &page))
        writeHeaderPage(page);
    }
    planes_[0].resize((size_t)info_.frame_width*info_.frame_height);
    planes_[1].resize((size_t)info_.frame_width/2*info_.frame_height/2);
    planes_[2].resize(planes_[1].size());
    return true;
  }
  bool hasSound() const {return sound_;}
  //! one picture of the constant frame rate; last for the final one
  void picture(const std::vector<unsigned char>& rgb, bool last){
    if (!rgb.empty())
      toYCbCr(rgb);
    th_ycbcr_buffer buffer;
    for (int i = 0; i < 3; ++i){
      buffer[i].width = i == 0 ? info_.frame_width : info_.frame_width/2;
      buffer[i].height = i == 0 ? info_.frame_height : info_.frame_height/2;
      buffer[i].stride = buffer[i].width;
      buffer[i].data = planes_[i].data();
    }
    th_encode_ycbcr_in(encoder_, buffer);
    ogg_packet packet;
    while (th_encode_packetout(encoder_, last ? 1 : 0, &packet) > 0)
      ogg_stream_packetin(&video_, &packet);
    pages(video_, last, true);
  }
  void sound(const std::vector<float>& samples, bool last){
    if (!sound_)
      return;
    size_t frames = samples.size()/channels_;
    //nothing written is the end of the sound
    if (frames == 0 && !last)
      return;
    if (frames > 0){
      float** buffer = vorbis_analysis_buffer(&dsp_, (int)frames);
      for (size_t i = 0; i < frames; ++i){
        for (int c = 0; c < channels_; ++c)
          buffer[c][i] = samples[i*channels_ + c];
      }
    }
    vorbis_analysis_wrote(&dsp_, last ? 0 : (int)frames);
    ogg_packet packet;
    while (vorbis_analysis_blockout(&dsp_, &block_) == 1){
      vorbis_analysis(&block_, nullptr);
      vorbis_bitrate_addblock(&block_);
      while (vorbis_bitrate_flushpacket(&dsp_, &packet))
        ogg_stream_packetin(&audio_, &packet);
    }
    pages(audio_, last, false);
  }
  //! the file: the headers, then the data pages of both streams by their times
  bool write(const std::wstring& path){
    std::stable_sort(data_.begin(), data_.end(), [](const Page& a, const Page& b){return a.time < b.time;});
    FILE* file = _wfopen(path.c_str(), L"wb");
    if (!file)
      return false;
    bool ok = fwrite(headers_.data(), 1, headers_.size(), file) == headers_.size();
    for (const Page& page : data_)
      ok = ok && fwrite(page.bytes.data(), 1, page.bytes.size(), file) == page.bytes.size();
    return fclose(file) == 0 && ok;
  }
private:
  struct Page{
    double time;
    std::vector<unsigned char> bytes;
  };
  void writeHeaderPage(const ogg_page& page){
    headers_.insert(headers_.end(), page.header, page.header+page.header_len);
    headers_.insert(headers_.end(), page.body, page.body+page.body_len);
  }
  void pages(ogg_stream_state& stream, bool flush, bool isVideo){
    ogg_page page;
    double& last = isVideo ? lastVideoTime_ : lastSoundTime_;
    while (flush ? ogg_stream_flush(&stream, &page) : ogg_stream_pageout(&stream, &page)){
      ogg_int64_t granule = ogg_page_granulepos(&page);
      //a page that ends no packet (-1) belongs after the one before
      if (granule >= 0)
        last = isVideo ? th_granule_time(encoder_, granule) : vorbis_granule_time(&dsp_, granule);
      Page p{last, {}};
      p.bytes.insert(p.bytes.end(), page.header, page.header+page.header_len);
      p.bytes.insert(p.bytes.end(), page.body, page.body+page.body_len);
      data_.push_back(std::move(p));
    }
  }
  //! BT.601 video range, the chroma of each 2x2 block averaged; the frame's padding repeats the edges
  void toYCbCr(const std::vector<unsigned char>& rgb){
    int fw = (int)info_.frame_width;
    int fh = (int)info_.frame_height;
    auto pixel = [&](int x, int y){
      x = std::min(x, width_-1);
      y = std::min(y, height_-1);
      return &rgb[((size_t)y*width_ + x)*3];
    };
    for (int y = 0; y < fh; ++y){
      for (int x = 0; x < fw; ++x){
        const unsigned char* p = pixel(x, y);
        planes_[0][(size_t)y*fw + x] = (unsigned char)((66*p[0] + 129*p[1] + 25*p[2] + 128 + 16*256) >> 8);
      }
    }
    for (int y = 0; y < fh/2; ++y){
      for (int x = 0; x < fw/2; ++x){
        int r = 0, g = 0, b = 0;
        for (int i = 0; i < 4; ++i){
          const unsigned char* p = pixel(2*x + i%2, 2*y + i/2);
          r += p[0]; g += p[1]; b += p[2];
        }
        planes_[1][(size_t)y*(fw/2) + x] = (unsigned char)std::clamp((-38*r - 74*g + 112*b + 512 + 128*1024) >> 10, 0, 255);
        planes_[2][(size_t)y*(fw/2) + x] = (unsigned char)std::clamp((112*r - 94*g - 18*b + 512 + 128*1024) >> 10, 0, 255);
      }
    }
  }
  int width_ = 0;
  int height_ = 0;
  th_info info_;
  th_comment comment_;
  th_enc_ctx* encoder_ = nullptr;
  ogg_stream_state video_;
  bool sound_ = false;
  int channels_ = 0;
  vorbis_info vorbis_;
  vorbis_comment vorbisComment_;
  vorbis_dsp_state dsp_;
  vorbis_block block_;
  ogg_stream_state audio_;
  std::vector<unsigned char> planes_[3];
  std::vector<unsigned char> headers_;
  std::vector<Page> data_;
  double lastVideoTime_ = 0;
  double lastSoundTime_ = 0;
};

std::vector<unsigned char> readFile(const std::wstring& path){
  std::vector<unsigned char> data;
  FILE* file = _wfopen(path.c_str(), L"rb");
  if (!file)
    return data;
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);
  if (size > 0){
    data.resize((size_t)size);
    data.resize(fread(data.data(), 1, data.size(), file));
  }
  fclose(file);
  return data;
}

//! does the Adventure play it as it is?
int check(const std::wstring& path){
  std::vector<unsigned char> data = readFile(path);
  if (data.empty())
    return 2;
  //MPEG-1 program stream ('0010' after the pack start code; MPEG-2's is '01'), with MP2 sound or none
  if (data.size() > 5 && data[0] == 0 && data[1] == 0 && data[2] == 1 && data[3] == 0xba && (data[4] & 0xf0) == 0x20){
    plm_t* plm = plm_create_with_memory(data.data(), data.size(), 0);
    bool ok = plm && plm_has_headers(plm) && plm_get_num_video_streams(plm) > 0 && plm_decode_video(plm) != nullptr;
    if (ok && plm_get_num_audio_streams(plm) > 0){
      //pl_mpeg decodes only layer II
      plm_rewind(plm);
      plm_set_video_enabled(plm, 0);
      ok = plm_decode_audio(plm) != nullptr;
    }
    if (plm)
      plm_destroy(plm);
    return ok ? 0 : 1;
  }
  //Ogg: each stream's first page names its codec; Theora is needed, Vorbis is the only sound
  if (data.size() > 4 && memcmp(data.data(), "OggS", 4) == 0){
    bool theora = false, other = false;
    size_t pos = 0;
    while (pos + 27 < data.size() && memcmp(&data[pos], "OggS", 4) == 0){
      unsigned char segments = data[pos+26];
      if (pos + 27 + segments > data.size())
        break;
      size_t body = 0;
      for (int i = 0; i < segments; ++i)
        body += data[pos+27+i];
      const unsigned char* packet = &data[pos+27+segments];
      bool bos = (data[pos+5] & 2) != 0;
      if (!bos)
        break;
      if (body >= 7 && memcmp(packet, "\x80theora", 7) == 0)
        theora = true;
      else if (!(body >= 7 && memcmp(packet, "\x01vorbis", 7) == 0) && !(body >= 8 && memcmp(packet, "fishead", 7) == 0))
        other = true;
      pos += 27 + segments + body;
    }
    return theora && !other ? 0 : 1;
  }
  return 1;
}

int convert(const std::wstring& in, const std::wstring& out, int quality){
  if (FAILED(MFStartup(MF_VERSION))){
    fprintf(stderr, "Media Foundation isn't available\n");
    return 2;
  }
  std::unique_ptr<Source> source;
  {
    auto mf = std::make_unique<MediaFoundationSource>();
    if (mf->open(in))
      source = std::move(mf);
  }
  if (!source){
    auto vfw = std::make_unique<VfwSource>();
    if (vfw->open(in))
      source = std::move(vfw);
  }
  if (!source){
    fprintf(stderr, "The video can't be decoded: Windows has no codec for it\n");
    MFShutdown();
    return 2;
  }
  int result = 0;
  {
    OgvWriter writer;
    if (!writer.open(*source, quality)){
      fprintf(stderr, "The encoder can't be set up for %ix%i\n", source->width, source->height);
      result = 2;
    }
    else{
      double fps = (double)source->fpsNum/source->fpsDen;
      std::vector<unsigned char> rgb, current;
      std::vector<float> sound;
      long long written = 0;
      bool havePicture = false;
      int lastPercent = -1;
      double time = 0;
      for (;;){
        Source::Kind kind = source->read(&time, rgb, sound);
        if (kind == Source::Kind::End)
          break;
        if (kind == Source::Kind::Sound){
          writer.sound(sound, false);
          continue;
        }
        //constant frame rate: the previous picture until this one's time
        long long index = std::max<long long>((long long)std::llround(time*fps), written);
        if (havePicture){
          while (written < index){
            writer.picture(current, false);
            ++written;
          }
        }
        if (!rgb.empty())
          current = rgb;
        havePicture = !current.empty();
        if (source->duration > 0){
          int percent = std::min(99, (int)(time*100/source->duration));
          if (percent != lastPercent){
            printf("progress %i\n", percent);
            fflush(stdout);
            lastPercent = percent;
          }
        }
      }
      if (havePicture){
        writer.picture(current, true);
        ++written;
      }
      writer.sound({}, true);
      if (written == 0){
        fprintf(stderr, "The video has no pictures\n");
        result = 2;
      }
      else if (!writer.write(out)){
        fprintf(stderr, "The output can't be written\n");
        result = 2;
      }
      else{
        printf("progress 100\n%ix%i, %.3f fps, %lld pictures, %s\n", source->width, source->height, fps, written,
          writer.hasSound() ? "with sound" : "without sound");
        //a codec unloading at the end may end the process before stdio does
        fflush(stdout);
      }
    }
  }
  source.reset();
  MFShutdown();
  return result;
}

}

int wmain(int argc, wchar_t** argv){
  if (argc == 3 && wcscmp(argv[1], L"--check") == 0)
    return check(argv[2]);
  int quality = 7;
  int arg = 1;
  if (argc == 5 && wcscmp(argv[1], L"-q") == 0){
    quality = _wtoi(argv[2]);
    arg = 3;
  }
  if (argc - arg != 2){
    fprintf(stderr, "videoconv --check <file>\nvideoconv [-q quality 0-10] <input> <output.ogv>\n");
    return 2;
  }
  CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  int result = convert(argv[arg], argv[arg+1], quality);
  CoUninitialize();
  return result;
}
