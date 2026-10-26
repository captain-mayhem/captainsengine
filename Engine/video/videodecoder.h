#ifndef CGE_VIDEODECODER_H
#define CGE_VIDEODECODER_H

#include <cge_export.h>

#include <memory>
#include <vector>

namespace CGE{

//! Decodes a video file in memory: its pictures and its sound
/*! Two formats: MPEG-1 video with MP2 sound in an MPEG program stream
    (.mpg, decoded with pl_mpeg), and Theora video with Vorbis sound in Ogg
    (.ogv, with libtheora and libvorbis). Other videos are converted into
    one of them beforehand (the Adventure's StoryDesigner does this when a
    video is added to a game).

    Pictures and sound are decoded independently, each in its own order;
    the caller times them (Video does this). Pictures come out as RGBA8,
    converted from the video's Y'CbCr (BT.601). */
class CGE_API VideoDecoder{
public:
  enum class Format{
    //! MPEG-1 video, MP2 sound, in an MPEG program stream
    Mpeg1,
    //! Theora video, Vorbis sound, in Ogg
    Theora,
  };

  //! opens a video; null if it isn't one of the formats or has no video
  static std::unique_ptr<VideoDecoder> open(std::vector<unsigned char> data);
  //! is this the start of a video in one of the formats?
  static bool isVideo(const unsigned char* data, size_t size);

  virtual ~VideoDecoder() = default;
  virtual Format getFormat() const = 0;
  //! the picture's size in pixels
  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
  //! the pictures per second
  virtual double getFrameRate() const = 0;
  //! has it sound?
  virtual bool hasAudio() const = 0;
  //! the sound's samples per second, and channels (interleaved)
  virtual int getSampleRate() const = 0;
  virtual int getChannels() const = 0;

  //! decodes the next picture; false at the end
  /*! time: when it is shown, in seconds from the start. The picture stays
      the decoder's until convertFrame, so pictures that are late can be
      skipped without converting them. */
  virtual bool decodeFrame(double* time) = 0;
  //! the last decoded picture as RGBA8, getWidth()*getHeight()*4 bytes, rows from the top
  virtual void convertFrame(unsigned char* rgba) = 0;
  //! decodes the next sound and appends its interleaved float samples; false at the end
  virtual bool decodeAudio(std::vector<float>& samples) = 0;
  //! back to the start of the pictures and the sound
  virtual bool rewind() = 0;
};

}

#endif
