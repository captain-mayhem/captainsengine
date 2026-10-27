// The video module: the MPEG-1 and Theora decoders, and the player timed by
// a mixer without a device.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include <gtest/gtest.h>

#include <theora/theoraenc.h>
#include <vorbis/vorbisenc.h>

#include <audio/audio.h>
#include <io/BinFileReader.h>
#include <video/video.h>
#include <video/videodecoder.h>

using CGE::Audio;
using CGE::AudioGroup;
using CGE::Video;
using CGE::VideoDecoder;

namespace{

const float PI = 3.14159265358979f;

struct Rgb{
  int r, g, b;
};

//! the four quadrants of the test videos, left to right, top to bottom
const Rgb QUADRANTS[4] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}};

std::vector<unsigned char> readFile(const char* name){
  CGE::BinFileReader reader(std::string(CGE_TEST_DATA_DIR) + "/" + name);
  std::vector<unsigned char> data(reader.getSize());
  if (!data.empty())
    reader.readBytes(data.data(), (unsigned)data.size());
  return data;
}

//! is the pixel at (x, y) of an RGBA picture this colour, within tolerance?
::testing::AssertionResult hasColour(const std::vector<unsigned char>& rgba, int width, int x, int y, Rgb colour, int tolerance = 16){
  const unsigned char* p = &rgba[((size_t)y*width + x)*4];
  if (std::abs(p[0]-colour.r) > tolerance || std::abs(p[1]-colour.g) > tolerance || std::abs(p[2]-colour.b) > tolerance || p[3] != 255)
    return ::testing::AssertionFailure() << "(" << x << ", " << y << ") is " << (int)p[0] << " " << (int)p[1] << " " << (int)p[2] << " " << (int)p[3]
      << ", not " << colour.r << " " << colour.g << " " << colour.b;
  return ::testing::AssertionSuccess();
}

//! checks the centres of the four quadrants
void expectQuadrants(const std::vector<unsigned char>& rgba, int width, int height){
  for (int i = 0; i < 4; ++i){
    int x = (i%2)*width/2 + width/4;
    int y = (i/2)*height/2 + height/4;
    EXPECT_TRUE(hasColour(rgba, width, x, y, QUADRANTS[i])) << "quadrant " << i;
  }
}

//! the frequency of a channel of interleaved samples, from its zero crossings
double frequency(const std::vector<float>& samples, int channels, int rate){
  size_t frames = samples.size()/channels;
  int crossings = 0;
  for (size_t i = 1; i < frames; ++i){
    if ((samples[(i-1)*channels] < 0) != (samples[i*channels] < 0))
      ++crossings;
  }
  return frames == 0 ? 0 : crossings/2.0/((double)frames/rate);
}

float rms(const std::vector<float>& samples, size_t begin = 0, size_t end = (size_t)-1){
  end = (std::min)(end, samples.size());
  if (end <= begin)
    return 0;
  double sum = 0;
  for (size_t i = begin; i < end; ++i)
    sum += samples[i]*samples[i];
  return (float)std::sqrt(sum/(end-begin));
}

//! a Theora video in Ogg, encoded with libtheora: the quadrants, and a sine in Vorbis if rate > 0
/*! The picture (picWidth x picHeight) sits at (1, 1) in frames rounded up
    to 16 pixels, so the decoder must crop it. */
std::vector<unsigned char> theora(int picWidth, int picHeight, int fps, int frames, int rate, float freq){
  std::vector<unsigned char> file;
  auto write = [&file](ogg_page& page){
    file.insert(file.end(), page.header, page.header+page.header_len);
    file.insert(file.end(), page.body, page.body+page.body_len);
  };
  th_info info;
  th_info_init(&info);
  info.frame_width = (picWidth+2+15) & ~15;
  info.frame_height = (picHeight+2+15) & ~15;
  info.pic_width = picWidth;
  info.pic_height = picHeight;
  info.pic_x = 1;
  info.pic_y = 1;
  info.fps_numerator = fps;
  info.fps_denominator = 1;
  info.aspect_numerator = 1;
  info.aspect_denominator = 1;
  info.colorspace = TH_CS_UNSPECIFIED;
  info.pixel_fmt = TH_PF_420;
  info.quality = 48;
  th_enc_ctx* encoder = th_encode_alloc(&info);
  EXPECT_NE(encoder, nullptr);
  th_comment comment;
  th_comment_init(&comment);
  ogg_stream_state video;
  ogg_stream_init(&video, 1);
  ogg_packet packet;
  ogg_page page;
  //the first header alone on the first page
  bool first = true;
  while (th_encode_flushheader(encoder, &comment, &packet) > 0){
    ogg_stream_packetin(&video, &packet);
    if (first && ogg_stream_flush(&video, &page))
      write(page);
    first = false;
  }

  bool sound = rate > 0;
  vorbis_info vorbis;
  vorbis_comment vorbisComment;
  vorbis_dsp_state dsp;
  vorbis_block block;
  ogg_stream_state audio;
  if (sound){
    vorbis_info_init(&vorbis);
    EXPECT_EQ(vorbis_encode_init_vbr(&vorbis, 2, rate, 0.4f), 0);
    vorbis_comment_init(&vorbisComment);
    vorbis_analysis_init(&dsp, &vorbis);
    vorbis_block_init(&dsp, &block);
    ogg_stream_init(&audio, 2);
    ogg_packet header, commentHeader, codeHeader;
    vorbis_analysis_headerout(&dsp, &vorbisComment, &header, &commentHeader, &codeHeader);
    ogg_stream_packetin(&audio, &header);
    //its first page right after Theora's, then the other headers of both
    if (ogg_stream_flush(&audio, &page))
      write(page);
    ogg_stream_packetin(&audio, &commentHeader);
    ogg_stream_packetin(&audio, &codeHeader);
  }
  while (ogg_stream_flush(&video, &page))
    write(page);
  while (sound && ogg_stream_flush(&audio, &page))
    write(page);

  //the pictures
  int chromaWidth = info.frame_width/2;
  int chromaHeight = info.frame_height/2;
  std::vector<unsigned char> planes[3] = {
    std::vector<unsigned char>(info.frame_width*info.frame_height),
    std::vector<unsigned char>(chromaWidth*chromaHeight),
    std::vector<unsigned char>(chromaWidth*chromaHeight)};
  for (int y = 0; y < (int)info.frame_height; ++y){
    for (int x = 0; x < (int)info.frame_width; ++x){
      int px = std::clamp(x-1, 0, picWidth-1);
      int py = std::clamp(y-1, 0, picHeight-1);
      Rgb c = QUADRANTS[(px >= picWidth/2 ? 1 : 0) + (py >= picHeight/2 ? 2 : 0)];
      //BT.601, video range
      double luma = 16 + (65.481*c.r + 128.553*c.g + 24.966*c.b)/255;
      double cb = 128 + (-37.797*c.r - 74.203*c.g + 112.0*c.b)/255;
      double cr = 128 + (112.0*c.r - 93.786*c.g - 18.214*c.b)/255;
      planes[0][y*info.frame_width + x] = (unsigned char)std::lround(luma);
      if (x%2 == 0 && y%2 == 0){
        planes[1][(y/2)*chromaWidth + x/2] = (unsigned char)std::lround(cb);
        planes[2][(y/2)*chromaWidth + x/2] = (unsigned char)std::lround(cr);
      }
    }
  }
  th_ycbcr_buffer buffer;
  for (int i = 0; i < 3; ++i){
    buffer[i].width = i == 0 ? info.frame_width : chromaWidth;
    buffer[i].height = i == 0 ? info.frame_height : chromaHeight;
    buffer[i].stride = buffer[i].width;
    buffer[i].data = planes[i].data();
  }
  for (int i = 0; i < frames; ++i){
    EXPECT_EQ(th_encode_ycbcr_in(encoder, buffer), 0);
    while (th_encode_packetout(encoder, i == frames-1, &packet) > 0)
      ogg_stream_packetin(&video, &packet);
  }

  //the sound, as long as the pictures
  if (sound){
    size_t total = (size_t)rate*frames/fps;
    for (size_t done = 0; ; ){
      size_t count = (std::min<size_t>)(1024, total-done);
      if (count == 0){
        vorbis_analysis_wrote(&dsp, 0);
      }
      else{
        float** buf = vorbis_analysis_buffer(&dsp, (int)count);
        for (size_t i = 0; i < count; ++i){
          float s = 0.5f*std::sin(2*PI*freq*(float)(done+i)/rate);
          buf[0][i] = s;
          buf[1][i] = s;
        }
        vorbis_analysis_wrote(&dsp, (int)count);
        done += count;
      }
      while (vorbis_analysis_blockout(&dsp, &block) == 1){
        vorbis_analysis(&block, nullptr);
        vorbis_bitrate_addblock(&block);
        while (vorbis_bitrate_flushpacket(&dsp, &packet))
          ogg_stream_packetin(&audio, &packet);
      }
      if (count == 0)
        break;
    }
  }

  //the pages of both streams, taking turns
  bool more = true;
  while (more){
    more = false;
    if (ogg_stream_pageout(&video, &page) || ogg_stream_flush(&video, &page)){
      write(page);
      more = true;
    }
    if (sound && (ogg_stream_pageout(&audio, &page) || ogg_stream_flush(&audio, &page))){
      write(page);
      more = true;
    }
  }

  ogg_stream_clear(&video);
  th_comment_clear(&comment);
  th_encode_free(encoder);
  th_info_clear(&info);
  if (sound){
    ogg_stream_clear(&audio);
    vorbis_block_clear(&block);
    vorbis_dsp_clear(&dsp);
    vorbis_comment_clear(&vorbisComment);
    vorbis_info_clear(&vorbis);
  }
  return file;
}

//! decodes all pictures; their times, and the first one's pixels
std::vector<double> decodeAll(VideoDecoder& decoder, std::vector<unsigned char>* first = nullptr){
  std::vector<double> times;
  double time;
  while (decoder.decodeFrame(&time)){
    if (times.empty() && first){
      first->resize((size_t)decoder.getWidth()*decoder.getHeight()*4);
      decoder.convertFrame(first->data());
    }
    times.push_back(time);
  }
  return times;
}

std::vector<float> decodeSound(VideoDecoder& decoder){
  std::vector<float> samples;
  while (decoder.decodeAudio(samples));
  return samples;
}

//! the pictures follow each other at the frame rate, from 0
void expectTimes(const std::vector<double>& times, double fps){
  ASSERT_FALSE(times.empty());
  EXPECT_NEAR(times[0], 0, 1e-6);
  for (size_t i = 1; i < times.size(); ++i)
    EXPECT_NEAR(times[i]-times[i-1], 1/fps, 1e-3) << "picture " << i;
}

}

TEST(VideoDecoder, RecognisesItsFormats){
  std::vector<unsigned char> mpeg = readFile("video/mpeg1.mpg");
  ASSERT_GT(mpeg.size(), 4u);
  EXPECT_TRUE(VideoDecoder::isVideo(mpeg.data(), mpeg.size()));
  std::vector<unsigned char> ogv = theora(30, 20, 10, 2, 0, 0);
  EXPECT_TRUE(VideoDecoder::isVideo(ogv.data(), ogv.size()));
  //an MPEG-2 pack header ('01' after the start code), an AVI, nothing
  const unsigned char mpeg2[] = {0, 0, 1, 0xba, 0x44, 0, 4, 0, 4, 1};
  EXPECT_FALSE(VideoDecoder::isVideo(mpeg2, sizeof(mpeg2)));
  EXPECT_EQ(VideoDecoder::open(std::vector<unsigned char>(mpeg2, mpeg2+sizeof(mpeg2))), nullptr);
  const unsigned char avi[] = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'A', 'V', 'I', ' '};
  EXPECT_EQ(VideoDecoder::open(std::vector<unsigned char>(avi, avi+sizeof(avi))), nullptr);
  EXPECT_EQ(VideoDecoder::open({}), nullptr);
  //an Ogg file without Theora (sound only)
  std::vector<unsigned char> click = readFile("audio/click.ogg");
  EXPECT_EQ(VideoDecoder::open(click), nullptr);
}

TEST(VideoDecoder, DecodesMpeg1){
  auto decoder = VideoDecoder::open(readFile("video/mpeg1.mpg"));
  ASSERT_NE(decoder, nullptr);
  EXPECT_EQ(decoder->getFormat(), VideoDecoder::Format::Mpeg1);
  EXPECT_EQ(decoder->getWidth(), 160);
  EXPECT_EQ(decoder->getHeight(), 120);
  EXPECT_DOUBLE_EQ(decoder->getFrameRate(), 25);
  ASSERT_TRUE(decoder->hasAudio());
  EXPECT_EQ(decoder->getSampleRate(), 44100);
  EXPECT_EQ(decoder->getChannels(), 2);

  std::vector<unsigned char> first;
  std::vector<double> times = decodeAll(*decoder, &first);
  EXPECT_NEAR(times.size(), 50, 2);
  expectTimes(times, 25);
  expectQuadrants(first, 160, 120);

  std::vector<float> sound = decodeSound(*decoder);
  EXPECT_NEAR(sound.size()/2/44100.0, 2, 0.1);
  EXPECT_NEAR(frequency(sound, 2, 44100), 440, 5);
  //a sine of 0.5: 0.35
  EXPECT_NEAR(rms(sound, 44100, 3*44100), 0.354, 0.03);

  //again from the start
  ASSERT_TRUE(decoder->rewind());
  std::vector<unsigned char> again;
  EXPECT_EQ(decodeAll(*decoder, &again).size(), times.size());
  EXPECT_EQ(again, first);
  EXPECT_EQ(decodeSound(*decoder).size(), sound.size());
}

TEST(VideoDecoder, DecodesTheoraWithVorbis){
  //an odd size inside 48x32 frames
  auto decoder = VideoDecoder::open(theora(45, 29, 15, 30, 22050, 330));
  ASSERT_NE(decoder, nullptr);
  EXPECT_EQ(decoder->getFormat(), VideoDecoder::Format::Theora);
  EXPECT_EQ(decoder->getWidth(), 45);
  EXPECT_EQ(decoder->getHeight(), 29);
  EXPECT_DOUBLE_EQ(decoder->getFrameRate(), 15);
  ASSERT_TRUE(decoder->hasAudio());
  EXPECT_EQ(decoder->getSampleRate(), 22050);
  EXPECT_EQ(decoder->getChannels(), 2);

  std::vector<unsigned char> first;
  std::vector<double> times = decodeAll(*decoder, &first);
  EXPECT_EQ(times.size(), 30u);
  expectTimes(times, 15);
  expectQuadrants(first, 45, 29);
  //the crop: the picture's corners are their quadrants' colours, not the frame's edge
  EXPECT_TRUE(hasColour(first, 45, 0, 0, QUADRANTS[0], 24));
  EXPECT_TRUE(hasColour(first, 45, 44, 28, QUADRANTS[3], 24));

  std::vector<float> sound = decodeSound(*decoder);
  EXPECT_NEAR(sound.size()/2/22050.0, 2, 0.05);
  EXPECT_NEAR(frequency(sound, 2, 22050), 330, 5);
  EXPECT_NEAR(rms(sound, 22050, 3*22050), 0.354, 0.03);

  ASSERT_TRUE(decoder->rewind());
  std::vector<unsigned char> again;
  EXPECT_EQ(decodeAll(*decoder, &again).size(), 30u);
  EXPECT_EQ(again, first);
}

TEST(VideoDecoder, DecodesTheoraWithoutSound){
  auto decoder = VideoDecoder::open(theora(64, 48, 25, 10, 0, 0));
  ASSERT_NE(decoder, nullptr);
  EXPECT_FALSE(decoder->hasAudio());
  EXPECT_EQ(decoder->getSampleRate(), 0);
  std::vector<float> sound;
  EXPECT_FALSE(decoder->decodeAudio(sound));
  std::vector<unsigned char> first;
  EXPECT_EQ(decodeAll(*decoder, &first).size(), 10u);
  expectQuadrants(first, 64, 48);
}

namespace{

const int RATE = 48000;

//! a mixer without a device; the tests mix it as a device would
class VideoTest : public ::testing::Test{
protected:
  void SetUp() override{
    Audio::Desc desc;
    desc.device = false;
    desc.sampleRate = RATE;
    ASSERT_TRUE(audio.init(desc));
  }
  //! mixes seconds of stereo and appends them to out
  void render(double seconds, std::vector<float>& out){
    size_t frames = (size_t)(seconds*RATE);
    size_t start = out.size();
    out.resize(start + frames*2);
    audio.render(&out[start], (unsigned)frames);
  }
  Audio audio;
};

}

TEST_F(VideoTest, PicturesFollowTheSound){
  auto video = Video::open(readFile("video/mpeg1.mpg"), &audio, AudioGroup::Sound);
  ASSERT_NE(video, nullptr);
  EXPECT_EQ(video->getWidth(), 160);
  EXPECT_EQ(video->getPixels(), nullptr);
  video->play();
  std::vector<float> mixed;
  //a device that plays 20 ms per frame of the application
  int steps = 0;
  unsigned lastFrames = 0;
  while (video->update(0.02) && steps < 500){
    render(0.02, mixed);
    ++steps;
    //the clock is what the mixer took (at most one block ahead of what was rendered)
    EXPECT_NEAR(video->getTime(), mixed.size()/2.0/RATE, 0.021);
    //the picture shown is the one for that time
    EXPECT_NEAR(video->getFrameCount(), video->getTime()*25 + 1, 2);
    EXPECT_GE(video->getFrameCount(), lastFrames);
    lastFrames = video->getFrameCount();
  }
  EXPECT_FALSE(video->isPlaying());
  //it ended with its sound (2 s), not before and not much later
  EXPECT_NEAR(mixed.size()/2.0/RATE, 2, 0.1);
  EXPECT_NEAR(lastFrames, 50, 2);
  ASSERT_NE(video->getPixels(), nullptr);
  //the tone came through the mixer, resampled to 48 kHz
  EXPECT_NEAR(frequency(mixed, 2, RATE), 440, 10);
  EXPECT_GT(rms(mixed, RATE/2, 3*RATE/2), 0.2f);

  //played again from the start
  video->play();
  EXPECT_TRUE(video->update(0));
  EXPECT_EQ(video->getFrameCount(), lastFrames+1);
  EXPECT_NEAR(video->getTime(), 0, 1e-9);
  video->stop();
  EXPECT_FALSE(video->update(0.02));
}

TEST_F(VideoTest, StopSilencesTheSound){
  auto video = Video::open(theora(32, 32, 25, 50, 44100, 440), &audio, AudioGroup::Sound);
  ASSERT_NE(video, nullptr);
  video->play();
  std::vector<float> mixed;
  for (int i = 0; i < 25; ++i){
    video->update(0.02);
    render(0.02, mixed);
  }
  EXPECT_GT(rms(mixed, 2*RATE/4), 0.1f);
  video->stop();
  EXPECT_FALSE(video->isPlaying());
  std::vector<float> after;
  render(0.02, after);
  after.clear();
  render(0.2, after);
  EXPECT_LT(rms(after), 1e-4f);
}

TEST_F(VideoTest, WithoutSoundTheTimePassedIsTheClock){
  auto video = Video::open(theora(32, 32, 10, 20, 0, 0), nullptr, AudioGroup::Sound);
  ASSERT_NE(video, nullptr);
  video->play();
  //a picture at once
  EXPECT_TRUE(video->update(0));
  EXPECT_EQ(video->getFrameCount(), 1u);
  EXPECT_TRUE(video->update(0.35));
  EXPECT_NEAR(video->getTime(), 0.35, 1e-9);
  //three late pictures skipped, one converted
  EXPECT_EQ(video->getFrameCount(), 2u);
  int steps = 0;
  while (video->update(0.05))
    ++steps;
  //2 s of pictures: the last one is shown for its 0.1 s
  EXPECT_NEAR(video->getTime(), 2.0, 0.051);
}

TEST_F(VideoTest, GoesOnWhenTheSoundDoesNotPlay){
  //the mixer isn't rendered, as on a web page before its first click
  auto video = Video::open(theora(32, 32, 10, 20, 22050, 440), &audio, AudioGroup::Sound);
  ASSERT_NE(video, nullptr);
  video->play();
  int steps = 0;
  while (video->update(0.05) && steps < 200)
    ++steps;
  EXPECT_FALSE(video->isPlaying());
  //half a second waiting for the sound, then the 2 s of pictures
  EXPECT_NEAR(steps*0.05, 2.5, 0.2);
}

TEST_F(VideoTest, ALongFrameIsNotAStall){
  auto video = Video::open(theora(32, 32, 10, 20, 22050, 440), &audio, AudioGroup::Sound);
  ASSERT_NE(video, nullptr);
  video->play();
  std::vector<float> mixed;
  video->update(0.02);
  render(0.02, mixed);
  //one frame of 0.8 s, in which the mixer took nothing more
  video->update(0.8);
  std::vector<float> after;
  for (int i = 0; i < 10; ++i){
    render(0.02, after);
    EXPECT_TRUE(video->update(0.02));
  }
  //still the sound's clock, and the sound still plays
  EXPECT_NEAR(video->getTime(), (mixed.size()+after.size())/2.0/RATE, 0.021);
  EXPECT_GT(rms(after), 0.2f);
}
