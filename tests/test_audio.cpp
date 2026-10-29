// The audio module, with a mixer without a device: render() mixes what the
// tests play, so the output can be measured.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <vorbis/vorbisenc.h>

#include <audio/audio.h>
#include <audio/audiodecoder.h>
#include <audio/audioeffect.h>
#include <audio/audioplayer.h>
#include <audio/audiostream.h>
#include <platform/assets.h>

using CGE::Audio;
using CGE::AudioDecoder;
using CGE::AudioEffect;
using CGE::AudioGroup;
using CGE::AudioPlayer;
using CGE::AudioStream;
using CGE::SoundId;

namespace{

const int RATE = 48000;
const float PI = 3.14159265358979f;
//! when a change is surely heard
const double SETTLE = 0.02;

//! a mixer without a device at 48 kHz
/*! miniaudio's nodes mix a little ahead, so a change (a pause, a new effect)
    is heard a few hundred frames later; the tests skip SETTLE seconds. */
class AudioTest : public ::testing::Test{
protected:
  void SetUp() override{
    Audio::Desc desc;
    desc.device = false;
    desc.sampleRate = RATE;
    ASSERT_TRUE(audio.init(desc));
  }
  //! mixes seconds of stereo
  std::vector<float> render(double seconds){
    std::vector<float> out((size_t)(seconds*RATE)*2);
    //in blocks, as a device would
    const unsigned block = 480;
    for (size_t frame = 0; frame < out.size()/2; frame += block){
      unsigned count = (unsigned)(std::min<size_t>)(block, out.size()/2-frame);
      audio.render(&out[frame*2], count);
    }
    audio.update();
    return out;
  }
  Audio audio;
};

//! the root mean square of the left channel of interleaved stereo, from first to last second
float rms(const std::vector<float>& stereo, double first = 0, double last = 1e9){
  size_t begin = (std::min)((size_t)(first*RATE), stereo.size()/2);
  size_t end = (std::min)((size_t)(last*RATE), stereo.size()/2);
  if (end <= begin)
    return 0;
  double sum = 0;
  for (size_t i = begin; i < end; ++i)
    sum += stereo[2*i]*stereo[2*i];
  return (float)std::sqrt(sum/(end-begin));
}

//! a sine as interleaved samples
std::vector<float> sine(int rate, int channels, double seconds, float freq, float amplitude){
  size_t frames = (size_t)(seconds*rate);
  std::vector<float> samples(frames*channels);
  for (size_t i = 0; i < frames; ++i){
    for (int c = 0; c < channels; ++c)
      samples[i*channels+c] = amplitude*std::sin(2*PI*freq*(float)i/rate);
  }
  return samples;
}

//! a 16-bit WAV file
std::vector<uint8_t> wav(int rate, int channels, const std::vector<float>& samples){
  std::vector<uint8_t> file;
  auto put = [&file](uint32_t value, int bytes){
    for (int i = 0; i < bytes; ++i)
      file.push_back((uint8_t)(value >> (8*i)));
  };
  uint32_t data = (uint32_t)samples.size()*2;
  file.insert(file.end(), {'R', 'I', 'F', 'F'});
  put(36+data, 4);
  file.insert(file.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
  put(16, 4);
  put(1, 2);
  put(channels, 2);
  put(rate, 4);
  put(rate*channels*2, 4);
  put(channels*2, 2);
  put(16, 2);
  file.insert(file.end(), {'d', 'a', 't', 'a'});
  put(data, 4);
  for (float s : samples)
    put((uint16_t)(int16_t)std::lround(std::clamp(s, -1.0f, 1.0f)*32767), 2);
  return file;
}

//! an Ogg Vorbis file, encoded with libvorbis
std::vector<uint8_t> ogg(int rate, int channels, const std::vector<float>& samples){
  std::vector<uint8_t> file;
  vorbis_info info;
  vorbis_info_init(&info);
  EXPECT_EQ(vorbis_encode_init_vbr(&info, channels, rate, 0.5f), 0);
  vorbis_comment comment;
  vorbis_comment_init(&comment);
  vorbis_dsp_state dsp;
  vorbis_analysis_init(&dsp, &info);
  vorbis_block block;
  vorbis_block_init(&dsp, &block);
  ogg_stream_state stream;
  ogg_stream_init(&stream, 1);
  ogg_page page;
  auto pages = [&](bool flush){
    while (flush ? ogg_stream_flush(&stream, &page) : ogg_stream_pageout(&stream, &page)){
      file.insert(file.end(), page.header, page.header+page.header_len);
      file.insert(file.end(), page.body, page.body+page.body_len);
    }
  };
  ogg_packet header, commentHeader, codeHeader;
  vorbis_analysis_headerout(&dsp, &comment, &header, &commentHeader, &codeHeader);
  ogg_stream_packetin(&stream, &header);
  ogg_stream_packetin(&stream, &commentHeader);
  ogg_stream_packetin(&stream, &codeHeader);
  pages(true);
  size_t frames = samples.size()/channels;
  size_t done = 0;
  for (bool end = false; !end;){
    size_t count = (std::min<size_t>)(1024, frames-done);
    if (count > 0){
      float** buffer = vorbis_analysis_buffer(&dsp, (int)count);
      for (size_t i = 0; i < count; ++i){
        for (int c = 0; c < channels; ++c)
          buffer[c][i] = samples[(done+i)*channels+c];
      }
      done += count;
    }
    vorbis_analysis_wrote(&dsp, (int)count);
    end = count == 0;
    while (vorbis_analysis_blockout(&dsp, &block) == 1){
      vorbis_analysis(&block, nullptr);
      vorbis_bitrate_addblock(&block);
      ogg_packet packet;
      while (vorbis_bitrate_flushpacket(&dsp, &packet)){
        ogg_stream_packetin(&stream, &packet);
        pages(false);
      }
    }
  }
  pages(true);
  ogg_stream_clear(&stream);
  vorbis_block_clear(&block);
  vorbis_dsp_clear(&dsp);
  vorbis_comment_clear(&comment);
  vorbis_info_clear(&info);
  return file;
}

//! interleaved float samples as bytes for a 32-bit stream
std::vector<float> silence(double seconds, int channels = 2){
  return std::vector<float>((size_t)(seconds*RATE)*channels, 0.0f);
}

//! the frame of the largest absolute sample of a channel from frame first on
size_t peak(const std::vector<float>& stereo, int channel, size_t first = 0){
  size_t best = first;
  for (size_t i = first; i < stereo.size()/2; ++i){
    if (std::fabs(stereo[2*i+channel]) > std::fabs(stereo[2*best+channel]))
      best = i;
  }
  return best;
}

//! the time a reverb's tail takes to fall by 60 dB, from its fall between -5 and -35 dB after its peak
double decayTime(const std::vector<float>& stereo, size_t start){
  const size_t window = RATE/100;
  std::vector<double> levels;
  for (size_t i = start; i+window <= stereo.size()/2; i += window){
    double sum = 0;
    for (size_t j = i; j < i+window; ++j)
      sum += stereo[2*j]*stereo[2*j] + stereo[2*j+1]*stereo[2*j+1];
    levels.push_back(10*std::log10(sum/window + 1e-30));
  }
  //from the peak, after the reverb built up
  size_t a = std::max_element(levels.begin(), levels.end()) - levels.begin();
  double top = levels[a];
  while (a < levels.size() && levels[a] > top-5)
    ++a;
  size_t b = a;
  while (b < levels.size() && levels[b] > top-35)
    ++b;
  if (b <= a)
    return 0;
  double seconds = (double)(b-a)*window/RATE;
  return seconds*60/30;
}

}

TEST(Audio, ClosedDoesNothing){
  Audio audio;
  EXPECT_FALSE(audio.isOpen());
  EXPECT_EQ(audio.getSampleRate(), 0);
  EXPECT_EQ(audio.loadSound("audio/sine.wav"), 0u);
  audio.playSound(1);
  EXPECT_FALSE(audio.playMusic("audio/sine.wav"));
  EXPECT_EQ(audio.createPlayer(std::vector<uint8_t>(10, 1)), nullptr);
  EXPECT_EQ(audio.createStream(), nullptr);
  EXPECT_EQ(audio.createEffect(), nullptr);
  float out[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  audio.render(out, 4);
  for (float s : out)
    EXPECT_EQ(s, 0.0f);
}

TEST_F(AudioTest, OpensWithoutDevice){
  EXPECT_TRUE(audio.isOpen());
  EXPECT_EQ(audio.getSampleRate(), RATE);
  EXPECT_EQ(audio.getBackendName(), "");
  EXPECT_FLOAT_EQ(audio.getMasterVolume(), 1.0f);
  //nothing plays: silence
  EXPECT_EQ(rms(render(0.1)), 0.0f);
}

TEST_F(AudioTest, DecodesWavMp3AndOggFiles){
  //WAV, MP3 and Ogg Vorbis (the last two from the Adventure's demo game)
  EXPECT_NE(audio.loadSound("audio/sine.wav"), 0u);
  EXPECT_NE(audio.loadSound("audio/beat.mp3"), 0u);
  EXPECT_NE(audio.loadSound("audio/click.ogg"), 0u);
  EXPECT_EQ(audio.loadSound("audio/missing.ogg"), 0u);
  std::vector<uint8_t> junk(1000, 0x55);
  EXPECT_EQ(audio.loadSound(junk.data(), junk.size()), 0u);
  std::vector<uint8_t> file;
  ASSERT_TRUE(CGE::Assets::load("audio/sine.wav", file));
  auto wavPlayer = audio.createPlayer(file);
  ASSERT_NE(wavPlayer, nullptr);
  EXPECT_NEAR(wavPlayer->getLength(), 0.5, 1e-4);
  ASSERT_TRUE(CGE::Assets::load("audio/beat.mp3", file));
  auto mp3Player = audio.createPlayer(file);
  ASSERT_NE(mp3Player, nullptr);
  //38 frames of 1152 samples at 44.1 kHz, less the decoder's delay
  EXPECT_NEAR(mp3Player->getLength(), 0.99, 0.03);
  ASSERT_TRUE(CGE::Assets::load("audio/click.ogg", file));
  auto oggPlayer = audio.createPlayer(file);
  ASSERT_NE(oggPlayer, nullptr);
  EXPECT_GT(oggPlayer->getLength(), 0.05);
  EXPECT_EQ(audio.createPlayer(junk), nullptr);
}

TEST_F(AudioTest, DecodesOggVorbis){
  //a second of a 1 kHz sine in stereo, at the mixer's rate
  std::vector<uint8_t> file = ogg(RATE, 2, sine(RATE, 2, 1.0, 1000, 0.5f));
  auto player = audio.createPlayer(file);
  ASSERT_NE(player, nullptr);
  EXPECT_NEAR(player->getLength(), 1.0, 1e-3);
  player->play();
  std::vector<float> out = render(0.5);
  //lossy, but close to the sine's 0.5/sqrt(2)
  EXPECT_NEAR(rms(out, 0.1, 0.5), 0.3536f, 0.02f);
  EXPECT_TRUE(player->isPlaying());
  EXPECT_NEAR(player->getPosition(), 0.5, 0.02);
}

TEST(AudioDecoder, DecodesWavMp3AndOgg){
  //without an Audio
  std::vector<float> samples = sine(22050, 1, 0.5, 440, 0.5f);
  auto decoder = AudioDecoder::open(wav(22050, 1, samples));
  ASSERT_NE(decoder, nullptr);
  EXPECT_EQ(decoder->getSampleRate(), 22050);
  EXPECT_EQ(decoder->getChannels(), 1);
  EXPECT_EQ(decoder->getLength(), samples.size());
  std::vector<float> out(samples.size()+100);
  ASSERT_EQ(decoder->read(out.data(), out.size()), samples.size());
  float worst = 0;
  for (size_t i = 0; i < samples.size(); ++i)
    worst = (std::max)(worst, std::fabs(out[i]-samples[i]));
  //16 bits
  EXPECT_LT(worst, 2.0f/32768);
  EXPECT_EQ(decoder->read(out.data(), 10), 0u);
  //back to the middle
  ASSERT_TRUE(decoder->seek(samples.size()/2));
  ASSERT_EQ(decoder->read(out.data(), 1), 1u);
  EXPECT_NEAR(out[0], samples[samples.size()/2], 2.0f/32768);

  //Ogg Vorbis in stereo, read in blocks
  decoder = AudioDecoder::open(ogg(RATE, 2, sine(RATE, 2, 1.0, 1000, 0.5f)));
  ASSERT_NE(decoder, nullptr);
  EXPECT_EQ(decoder->getSampleRate(), RATE);
  EXPECT_EQ(decoder->getChannels(), 2);
  EXPECT_EQ(decoder->getLength(), (unsigned long long)RATE);
  std::vector<float> all;
  std::vector<float> block(1000*2);
  for (size_t got; (got = decoder->read(block.data(), 1000)) > 0;)
    all.insert(all.end(), block.begin(), block.begin()+got*2);
  EXPECT_EQ(all.size()/2, (size_t)RATE);
  //lossy, but close to the sine's 0.5/sqrt(2)
  EXPECT_NEAR(rms(all, 0.1, 0.9), 0.3536f, 0.02f);

  //an MP3 (from the Adventure's demo game): its length, read through before decoding, is what decodes
  std::vector<uint8_t> file;
  ASSERT_TRUE(CGE::Assets::load("audio/beat.mp3", file));
  decoder = AudioDecoder::open(file);
  ASSERT_NE(decoder, nullptr);
  EXPECT_EQ(decoder->getSampleRate(), 44100);
  unsigned long long length = decoder->getLength();
  EXPECT_NEAR((double)length/44100, 0.99, 0.03);
  std::vector<float> mp3(((size_t)length+1000)*decoder->getChannels());
  EXPECT_EQ(decoder->read(mp3.data(), (size_t)length+1000), length);
  EXPECT_GT(*(std::max_element)(mp3.begin(), mp3.end()), 0.1f);

  EXPECT_EQ(AudioDecoder::open(std::vector<uint8_t>(1000, 0x55)), nullptr);
  EXPECT_EQ(AudioDecoder::open(std::vector<uint8_t>()), nullptr);
}

TEST_F(AudioTest, PlaySoundAtVolumeAndPitch){
  SoundId id = audio.loadSound("audio/sine.wav");
  ASSERT_NE(id, 0u);
  audio.playSound(id);
  EXPECT_EQ(audio.getPlayingSounds(), 1);
  std::vector<float> full = render(0.4);
  //a mono sine of 0.5 at 44.1 kHz, the same on both sides
  float level = rms(full, 0.05, 0.4);
  EXPECT_NEAR(level, 0.3536f, 0.01f);
  EXPECT_NEAR(full[2*1000], full[2*1000+1], 1e-6f);
  //it ends after half a second and is freed
  render(0.2);
  EXPECT_EQ(audio.getPlayingSounds(), 0);

  audio.playSound(id, 0.5f);
  EXPECT_NEAR(rms(render(0.4), 0.05, 0.4), level*0.5f, 0.005f);
  render(0.2);

  //twice as fast: gone after a quarter of a second
  audio.playSound(id, 1, 2);
  render(0.2);
  EXPECT_EQ(audio.getPlayingSounds(), 1);
  render(0.1);
  EXPECT_EQ(audio.getPlayingSounds(), 0);

  //the same sound twice at once adds up
  audio.playSound(id);
  audio.playSound(id);
  EXPECT_NEAR(rms(render(0.3), 0.05, 0.3), level*2, 0.02f);
  audio.stopSounds();
  EXPECT_EQ(audio.getPlayingSounds(), 0);
  EXPECT_EQ(rms(render(0.1)), 0.0f);

  //unloaded while it plays, it plays to its end
  audio.playSound(id);
  audio.unloadSound(id);
  EXPECT_NEAR(rms(render(0.3), 0.05, 0.3), level, 0.01f);
  audio.playSound(id);
  render(0.3);
  EXPECT_EQ(audio.getPlayingSounds(), 0);
}

TEST_F(AudioTest, VolumesOfGroupsAndMaster){
  SoundId id = audio.loadSound("audio/sine.wav");
  ASSERT_NE(id, 0u);
  audio.playSound(id);
  float level = rms(render(0.3), 0.05, 0.3);
  audio.stopSounds();
  audio.setVolume(AudioGroup::Sound, 0.5f);
  EXPECT_FLOAT_EQ(audio.getVolume(AudioGroup::Sound), 0.5f);
  EXPECT_FLOAT_EQ(audio.getVolume(AudioGroup::Music), 1.0f);
  audio.playSound(id);
  EXPECT_NEAR(rms(render(0.3), 0.05, 0.3), level*0.5f, 0.005f);
  audio.stopSounds();
  audio.setMasterVolume(0.5f);
  EXPECT_FLOAT_EQ(audio.getMasterVolume(), 0.5f);
  audio.playSound(id);
  EXPECT_NEAR(rms(render(0.3), 0.05, 0.3), level*0.25f, 0.005f);
  audio.stopSounds();
  //the music group isn't affected by the sound group's volume
  audio.setMasterVolume(1);
  ASSERT_TRUE(audio.playMusic("audio/sine.wav"));
  EXPECT_NEAR(rms(render(0.3), 0.05, 0.3), level, 0.01f);
  audio.setVolume(AudioGroup::Music, 0);
  EXPECT_LT(rms(render(0.2), 0.05, 0.2), 0.001f);
}

TEST_F(AudioTest, MusicLoopsFadesAndCrossfades){
  EXPECT_FALSE(audio.playMusic("audio/missing.ogg"));
  EXPECT_FALSE(audio.isMusicPlaying());
  ASSERT_TRUE(audio.playMusic("audio/sine.wav", true, 0.4f));
  EXPECT_TRUE(audio.isMusicPlaying());
  std::vector<float> out = render(1.5);
  //it fades in over 0.4 s, and loops the half-second file
  float full = rms(out, 1.2, 1.5);
  EXPECT_NEAR(full, 0.3536f, 0.01f);
  EXPECT_LT(rms(out, 0, 0.1), full*0.4f);
  EXPECT_TRUE(audio.isMusicPlaying());
  //the same piece goes on
  EXPECT_TRUE(audio.playMusic("audio/sine.wav", true, 0.4f));
  EXPECT_NEAR(rms(render(0.1)), full, 0.01f);
  //a crossfade to another piece: both play while it lasts
  ASSERT_TRUE(audio.playMusic("audio/click.ogg", false, 0.3f));
  render(0.5);
  EXPECT_TRUE(audio.isMusicPlaying());
  //the click ends by itself, and the old piece faded out
  render(1.0);
  EXPECT_FALSE(audio.isMusicPlaying());
  EXPECT_LT(rms(render(0.2)), 0.001f);
  //a fade out
  ASSERT_TRUE(audio.playMusic("audio/sine.wav"));
  render(0.2);
  audio.stopMusic(0.2f);
  EXPECT_TRUE(audio.isMusicPlaying());
  out = render(0.4);
  EXPECT_FALSE(audio.isMusicPlaying());
  EXPECT_LT(rms(out, 0.25, 0.4), 0.001f);
  //at once
  ASSERT_TRUE(audio.playMusic("audio/sine.wav"));
  render(0.1);
  audio.stopMusic();
  EXPECT_FALSE(audio.isMusicPlaying());
}

TEST_F(AudioTest, PlayerPlaysPausesStopsAndSeeks){
  std::vector<uint8_t> file = wav(RATE, 1, sine(RATE, 1, 1.0, 500, 0.5f));
  auto player = audio.createPlayer(file);
  ASSERT_NE(player, nullptr);
  EXPECT_FALSE(player->isPlaying());
  EXPECT_NEAR(player->getLength(), 1.0, 1e-6);
  EXPECT_EQ(rms(render(0.1)), 0.0f);
  player->play();
  EXPECT_TRUE(player->isPlaying());
  EXPECT_NEAR(rms(render(0.3)), 0.3536f, 0.01f);
  EXPECT_NEAR(player->getPosition(), 0.3, 0.01);
  player->pause();
  EXPECT_FALSE(player->isPlaying());
  EXPECT_EQ(rms(render(0.1), SETTLE), 0.0f);
  EXPECT_NEAR(player->getPosition(), 0.3, 0.01);
  player->play();
  render(0.1);
  EXPECT_NEAR(player->getPosition(), 0.4, 0.01);
  //to the end: it stops
  player->seek(0.9);
  render(0.2);
  EXPECT_FALSE(player->isPlaying());
  //playing again starts at the beginning
  player->play();
  render(0.1);
  EXPECT_NEAR(player->getPosition(), 0.1, 0.01);
  //stop rewinds
  player->stop();
  EXPECT_FALSE(player->isPlaying());
  player->play();
  render(0.2);
  EXPECT_NEAR(player->getPosition(), 0.2, 0.01);
  //looping goes on past the end
  player->stop();
  player->play(true);
  EXPECT_TRUE(player->isLooping());
  std::vector<float> out = render(2.5);
  EXPECT_TRUE(player->isPlaying());
  EXPECT_NEAR(rms(out, 2.2, 2.5), 0.3536f, 0.01f);
  player->setVolume(0.5f);
  EXPECT_FLOAT_EQ(player->getVolume(), 0.5f);
  EXPECT_NEAR(rms(render(0.2), 0.05, 0.2), 0.1768f, 0.01f);
  //twice the pitch: half a second of the file in a quarter
  player->stop();
  player->setPitch(2);
  player->play();
  render(0.25);
  EXPECT_NEAR(player->getPosition(), 0.5, 0.02);
}

TEST_F(AudioTest, StreamQueuesPlaysAndRunsDry){
  auto stream = audio.createStream();
  ASSERT_NE(stream, nullptr);
  EXPECT_FALSE(stream->isOpen());
  EXPECT_FALSE(stream->open(RATE, 24, 2));
  ASSERT_TRUE(stream->open(RATE, 16, 2));
  EXPECT_TRUE(stream->isOpen());
  //0.1 s of a 1 kHz sine: less than the quarter second it waits for
  std::vector<float> wave = sine(RATE, 2, 0.6, 1000, 0.5f);
  std::vector<int16_t> pcm(wave.size());
  for (size_t i = 0; i < wave.size(); ++i)
    pcm[i] = (int16_t)std::lround(wave[i]*32767);
  const size_t frame = 2*sizeof(int16_t);
  size_t first = (size_t)(0.1*RATE)*frame;
  EXPECT_EQ(stream->tryWrite(pcm.data(), first), first);
  EXPECT_EQ(stream->getQueuedBytes(), first);
  EXPECT_EQ(rms(render(0.05)), 0.0f);
  EXPECT_EQ(stream->getQueuedBytes(), first);
  //now 0.6 s: it plays, and runs dry after it
  size_t rest = pcm.size()*sizeof(int16_t)-first;
  EXPECT_EQ(stream->tryWrite((const char*)pcm.data()+first, rest), rest);
  std::vector<float> out = render(0.8);
  EXPECT_NEAR(rms(out, 0.05, 0.55), 0.3536f, 0.01f);
  EXPECT_EQ(rms(out, 0.65, 0.8), 0.0f);
  EXPECT_EQ(stream->getQueuedBytes(), 0u);
  EXPECT_GT(stream->getUnderruns(), 0u);
  //a full buffer takes no more than a second
  std::vector<int16_t> lots((size_t)(1.5*RATE)*2, 1000);
  EXPECT_EQ(stream->tryWrite(lots.data(), lots.size()*sizeof(int16_t)), (size_t)RATE*frame);
  stream->setVolume(0.5f);
  EXPECT_NEAR(rms(render(0.2), 0.05, 0.2), 0.5f*1000/32768, 0.001f);
  //halted, it drops what is queued and takes nothing
  stream->halt();
  EXPECT_EQ(rms(render(0.1), SETTLE), 0.0f);
  EXPECT_EQ(stream->tryWrite(pcm.data(), first), 0u);
  stream->close();
  EXPECT_FALSE(stream->isOpen());
}

TEST_F(AudioTest, StreamWriteAndDrainFromAThread){
  auto stream = audio.createStream(AudioGroup::Music);
  ASSERT_NE(stream, nullptr);
  //mono float at 24 kHz: converted to the mixer's format and rate
  ASSERT_TRUE(stream->open(24000, 32, 1));
  std::vector<float> wave = sine(24000, 1, 2.0, 440, 0.5f);
  std::atomic<bool> done{false};
  //a decoder's thread: two seconds of samples through a buffer of one, then drained
  std::thread decoder([&]{
    stream->write(wave.data(), wave.size()*sizeof(float));
    stream->drain();
    done = true;
  });
  std::vector<float> out;
  for (int i = 0; i < 400 && !done; ++i){
    std::vector<float> part = render(0.01);
    out.insert(out.end(), part.begin(), part.end());
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  decoder.join();
  ASSERT_TRUE(done);
  //all of it played, at the sine's level
  EXPECT_GE(out.size()/2, (size_t)(2.0*RATE));
  EXPECT_NEAR(rms(out, 0.1, 1.9), 0.3536f, 0.01f);
  EXPECT_EQ(stream->getQueuedBytes(), 0u);
  //short sounds play at drain, below the start threshold
  ASSERT_TRUE(stream->open(RATE, 32, 2));
  std::vector<float> beep = sine(RATE, 2, 0.1, 1000, 0.5f);
  EXPECT_EQ(stream->tryWrite(beep.data(), beep.size()*sizeof(float)), beep.size()*sizeof(float));
  std::thread drainer([&]{stream->drain();});
  out.clear();
  for (int i = 0; i < 100 && stream->getQueuedBytes() > 0; ++i){
    std::vector<float> part = render(0.01);
    out.insert(out.end(), part.begin(), part.end());
  }
  drainer.join();
  std::vector<float> end = render(0.05);
  out.insert(out.end(), end.begin(), end.end());
  //all of it played: the energy of 0.1 s of the sine
  double energy = 0;
  for (size_t i = 0; i < out.size(); i += 2)
    energy += out[i]*out[i];
  EXPECT_NEAR(energy/(0.1*RATE), 0.125, 0.01);
}

TEST_F(AudioTest, StreamsOpenAndCloseOnManyThreads){
  //as guit3 does: each sound's decoder thread opens its stream, plays it out and closes it, while others do the same
  const int threads = 4;
  std::vector<std::unique_ptr<AudioStream>> streams;
  for (int t = 0; t < threads; ++t)
    streams.push_back(audio.createStream());
  std::atomic<bool> mixing{true};
  //the device's thread
  std::thread mixer([&]{
    std::vector<float> out(480*2);
    while (mixing){
      audio.render(out.data(), 480);
      std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
  });
  std::atomic<int> sounds{0};
  std::vector<std::thread> decoders;
  for (int t = 0; t < threads; ++t){
    decoders.emplace_back([&, t]{
      std::vector<float> beep = sine(RATE, 2, 0.005, 1000, 0.1f);
      for (int i = 0; i < 50; ++i){
        AudioStream& stream = *streams[t];
        if (!stream.open(RATE, 32, 2))
          continue;
        stream.write(beep.data(), beep.size()*sizeof(float));
        if ((i+t)%3 == 0)
          stream.halt();
        else
          stream.drain();
        stream.close();
        ++sounds;
      }
    });
  }
  for (std::thread& decoder : decoders)
    decoder.join();
  mixing = false;
  mixer.join();
  EXPECT_EQ(sounds, threads*50);
  //the mixer still plays
  ASSERT_TRUE(streams[0]->open(RATE, 32, 2));
  std::vector<float> wave = sine(RATE, 2, 0.5, 1000, 0.5f);
  EXPECT_EQ(streams[0]->tryWrite(wave.data(), wave.size()*sizeof(float)), wave.size()*sizeof(float));
  EXPECT_NEAR(rms(render(0.4), 0.05, 0.4), 0.3536f, 0.01f);
}

TEST_F(AudioTest, EffectWithoutEffectIsDry){
  auto effect = audio.createEffect();
  ASSERT_NE(effect, nullptr);
  auto stream = audio.createStream();
  ASSERT_TRUE(stream->open(RATE, 32, 2));
  stream->setEffect(effect.get());
  std::vector<float> wave = sine(RATE, 2, 0.5, 1000, 0.5f);
  stream->tryWrite(wave.data(), wave.size()*sizeof(float));
  EXPECT_NEAR(rms(render(0.4), 0.05, 0.4), 0.3536f, 0.005f);
  effect->setMix(0.5f, 1);
  EXPECT_NEAR(rms(render(0.05), SETTLE), 0.1768f, 0.005f);
  //directly to the group again
  stream->setEffect(nullptr);
  EXPECT_NEAR(rms(render(0.05), SETTLE), 0.3536f, 0.005f);
}

TEST_F(AudioTest, ReverbDecaysInItsDecayTime){
  auto effect = audio.createEffect();
  ASSERT_NE(effect, nullptr);
  for (float decay : {0.8f, 2.5f}){
    CGE::ReverbParams params;
    params.decayTime = decay;
    params.decayHFRatio = 1;
    params.gainHF = 1;
    params.reflectionsGain = 0;
    effect->setReverb(params);
    effect->setMix(0, 1);
    //50 ms of noise, then silence
    auto stream = audio.createStream();
    ASSERT_TRUE(stream->open(RATE, 32, 2));
    stream->setEffect(effect.get());
    std::vector<float> burst = silence(0.9);
    unsigned seed = 1;
    for (size_t i = 0; i < (size_t)(0.05*RATE)*2; ++i){
      seed = seed*1664525u + 1013904223u;
      burst[i] = ((seed >> 8)/(float)(1 << 24) - 0.5f);
    }
    stream->tryWrite(burst.data(), burst.size()*sizeof(float));
    std::vector<float> out = render(decay*1.5f + 0.3f);
    double measured = decayTime(out, (size_t)(0.05*RATE));
    EXPECT_NEAR(measured, decay, decay*0.25) << "decay time " << decay;
    //the reverb gives about the energy that goes in (the gains are 0.32 * 1.26)
    EXPECT_GT(rms(out, 0.05, 0.3), 0.01f);
    //the tail goes on after the stream ended
    stream.reset();
    EXPECT_GT(rms(render(0.1)), 0.0f);
    render(decay*2);
  }
}

TEST_F(AudioTest, ReverbPresetsStayStableAndDecay){
  //EFX presets the Adventure uses (EFXeffects.h): a cave, mountains with an
  //echo, and two with a strong modulation and little density
  struct Preset{
    const char* name;
    CGE::ReverbParams params;
  };
  auto preset = [](float density, float diffusion, float gainHF, float decay, float hfRatio, float reflGain, float reflDelay,
    float lateGain, float lateDelay, float echoTime, float echoDepth, float modTime, float modDepth){
    CGE::ReverbParams p;
    p.density = density; p.diffusion = diffusion; p.gain = 0.316228f; p.gainHF = gainHF;
    p.decayTime = decay; p.decayHFRatio = hfRatio; p.reflectionsGain = reflGain; p.reflectionsDelay = reflDelay;
    p.lateReverbGain = lateGain; p.lateReverbDelay = lateDelay; p.echoTime = echoTime; p.echoDepth = echoDepth;
    p.modulationTime = modTime; p.modulationDepth = modDepth;
    return p;
  };
  const Preset presets[] = {
    {"cave", preset(1, 1, 1, 2.91f, 1.3f, 0.500035f, 0.015f, 0.706318f, 0.022f, 0.25f, 0, 0.25f, 0)},
    {"mountains", preset(1, 0.27f, 0.0562341f, 1.49f, 0.21f, 0.040738f, 0.3f, 0.191867f, 0.1f, 0.25f, 1, 0.25f, 0)},
    {"drugged", preset(0.428687f, 0.5f, 1, 8.39f, 1.39f, 0.875992f, 0.002f, 3.10814f, 0.03f, 0.25f, 0, 0.25f, 1)},
    {"psychotic", preset(0.0625f, 0.5f, 0.840427f, 7.56f, 0.91f, 0.486407f, 0.02f, 2.43781f, 0.03f, 0.25f, 0, 4, 1)},
  };
  auto effect = audio.createEffect();
  ASSERT_NE(effect, nullptr);
  effect->setMix(0, 1);
  for (const Preset& preset : presets){
    effect->setNone();
    render(0.05);
    effect->setReverb(preset.params);
    auto stream = audio.createStream();
    ASSERT_TRUE(stream->open(RATE, 32, 2));
    stream->setEffect(effect.get());
    //50 ms of noise
    std::vector<float> burst = silence(0.9);
    unsigned seed = 7;
    for (size_t i = 0; i < (size_t)(0.05*RATE)*2; ++i){
      seed = seed*1664525u + 1013904223u;
      burst[i] = ((seed >> 8)/(float)(1 << 24) - 0.5f);
    }
    stream->tryWrite(burst.data(), burst.size()*sizeof(float));
    float decay = preset.params.decayTime;
    std::vector<float> out = render((std::min)(decay*1.2f, 6.0f) + 0.5f);
    stream.reset();
    bool finite = true;
    float peak = 0;
    for (float v : out){
      finite = finite && std::isfinite(v);
      peak = (std::max)(peak, std::fabs(v));
    }
    EXPECT_TRUE(finite) << preset.name;
    EXPECT_LT(peak, 2.0f) << preset.name;
    EXPECT_GT(rms(out, 0.05, 0.5), 0.001f) << preset.name;
    //the high frequencies decay faster (decayHFRatio), so the measured time is at most the decay time
    double measured = decayTime(out, (size_t)(0.05*RATE));
    RecordProperty(preset.name, std::to_string(measured));
    EXPECT_GT(measured, decay*0.5) << preset.name;
    EXPECT_LT(measured, decay*1.25) << preset.name;
    effect->setNone();
    render(0.1);
  }
}

TEST_F(AudioTest, EchoRepeatsAfterItsDelays){
  auto effect = audio.createEffect();
  ASSERT_NE(effect, nullptr);
  CGE::EchoParams params;
  params.delay = 0.1f;
  params.lrDelay = 0.1f;
  params.feedback = 0;
  params.spread = -1;
  effect->setEcho(params);
  effect->setMix(1, 1);
  auto stream = audio.createStream();
  ASSERT_TRUE(stream->open(RATE, 32, 2));
  stream->setEffect(effect.get());
  std::vector<float> impulse = silence(0.6);
  impulse[0] = impulse[1] = 1;
  stream->tryWrite(impulse.data(), impulse.size()*sizeof(float));
  std::vector<float> out = render(0.5);
  //the dry impulse, then the first tap on the left and the second on the right
  size_t dry = peak(out, 0);
  size_t left = peak(out, 0, dry+100);
  size_t right = peak(out, 1, dry+100);
  EXPECT_NEAR((double)(left-dry)/RATE, 0.1, 0.002);
  EXPECT_NEAR((double)(right-dry)/RATE, 0.2, 0.002);
  EXPECT_GT(std::fabs(out[2*left]), 0.3f);
  EXPECT_LT(std::fabs(out[2*right]), 0.01f);
}

TEST_F(AudioTest, ChorusAndDistortionChangeTheSound){
  auto effect = audio.createEffect();
  ASSERT_NE(effect, nullptr);
  auto stream = audio.createStream();
  ASSERT_TRUE(stream->open(RATE, 32, 2));
  stream->setEffect(effect.get());
  std::vector<float> wave = sine(RATE, 2, 1.0, 440, 0.8f);
  stream->tryWrite(wave.data(), wave.size()*sizeof(float));
  effect->setMix(0, 1);
  //a chorus: the delayed sine, about as loud
  effect->setChorus(CGE::ChorusParams());
  std::vector<float> chorus = render(0.3);
  EXPECT_NEAR(rms(chorus, 0.1, 0.3), 0.5657f, 0.15f);
  //a flanger with feedback
  effect->setChorus(CGE::ChorusParams::flanger());
  std::vector<float> flanger = render(0.3);
  EXPECT_GT(rms(flanger, 0.1, 0.3), 0.1f);
  //a distortion: the band-passed, clipped sine at its gain
  CGE::DistortionParams distortion;
  distortion.edge = 0.8f;
  distortion.gain = 0.5f;
  effect->setDistortion(distortion);
  std::vector<float> distorted = render(0.3);
  float level = rms(distorted, 0.1, 0.3);
  EXPECT_GT(level, 0.05f);
  for (size_t i = (size_t)(SETTLE*RATE)*2; i < distorted.size(); ++i)
    ASSERT_LE(std::fabs(distorted[i]), 0.6f) << i;
}
