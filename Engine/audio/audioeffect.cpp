// The effect bus: a miniaudio node that mixes its input dry and through one
// of the effects (reverb, echo, chorus/flanger, distortion). The effects
// approximate OpenAL's EFX effects of the same names; their parameters are
// handed to the audio thread under a spinlock and applied at its next block.

#include "audioeffect.h"
#include "audio_internal.h"

#include <algorithm>
#include <cmath>

using namespace CGE;

namespace{

const float PI = 3.14159265358979f;
//! the reference frequency of EFX's high-frequency parameters
const float HF_REFERENCE = 5000;

//! a delay line; tap(d) is the sample written d samples ago (d >= 1)
struct DelayLine{
  std::vector<float> buf;
  size_t pos = 0;

  void init(size_t length){buf.assign(length < 2 ? 2 : length, 0.0f); pos = 0;}
  void clear(){std::fill(buf.begin(), buf.end(), 0.0f);}
  void write(float x){
    buf[pos] = x;
    if (++pos == buf.size())
      pos = 0;
  }
  float tap(size_t d) const{
    d = std::min(std::max<size_t>(d, 1), buf.size());
    return buf[(pos+buf.size()-d) % buf.size()];
  }
  //! with linear interpolation between samples
  float tap(float d) const{
    d = std::min(std::max(d, 1.0f), (float)buf.size()-1);
    size_t i = (size_t)d;
    float frac = d-(float)i;
    return tap(i)*(1-frac) + tap(i+1)*frac;
  }
};

//! the pole of a one-pole low-pass (y = (1-p)x + p*y') whose gain is gain at
//! the frequency w with cos(w) = c
float onePolePoleCos(float gain, float c){
  if (gain >= 0.9999f)
    return 0;
  gain = std::max(gain, 0.0001f);
  float a2 = gain*gain;
  //(1-p)^2 = a^2 (1 - 2p cos w + p^2), the root in [0, 1)
  float b = 1-a2*c;
  float disc = b*b - (1-a2)*(1-a2);
  return (b - std::sqrt(std::max(disc, 0.0f)))/(1-a2);
}

//! the same at freq
float onePolePole(float gain, float freq, float sampleRate){
  return onePolePoleCos(gain, std::cos(2*PI*std::min(freq, sampleRate*0.49f)/sampleRate));
}

//! Freeverb's comb filter with a damped feedback
struct Comb{
  DelayLine line;
  size_t length = 1;
  size_t idx = 0;
  float store = 0;
  float feedback = 0;
  float damp = 0;
  float inGain = 1;

  float process(float x){
    float out = line.buf[idx];
    store = out*(1-damp) + store*damp;
    line.buf[idx] = x*inGain + store*feedback;
    if (++idx >= length)
      idx = 0;
    return out;
  }
  void clear(){line.clear(); store = 0;}
};

//! Freeverb's allpass
struct Allpass{
  DelayLine line;
  size_t idx = 0;
  float feedback = 0.5f;

  float process(float x){
    float buffered = line.buf[idx];
    line.buf[idx] = x + buffered*feedback;
    if (++idx >= line.buf.size())
      idx = 0;
    return buffered - x;
  }
};

//! Freeverb's tunings at 44.1 kHz, the right side's combs and allpasses spread by 23 samples
const int COMB_TUNING[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
const int ALLPASS_TUNING[4] = {556, 441, 341, 225};
const int STEREO_SPREAD = 23;

struct Reverb{
  float rate = 48000;
  //! the input before the reflections and the late reverb
  DelayLine predelay;
  //! EFX's echo in the late reverb
  DelayLine echo;
  Comb combs[2][8];
  Allpass allpasses[2][4];
  size_t combMax[2][8];
  //! the high-frequency loss of the output (gainHF)
  float hfPole = 0;
  float hfStore[2] = {0, 0};

  //! the derived parameters
  float reflDelay = 0;
  float lateDelay = 0;
  float reflGain = 0;
  float lateGain = 0;
  float echoDelay = 0;
  float echoGain = 0;
  float modPhase = 0;
  float modStep = 0;
  float modDepth = 0;

  void init(float sampleRate){
    rate = sampleRate;
    float scale = sampleRate/44100.0f;
    predelay.init((size_t)(0.5f*sampleRate)+2);
    echo.init((size_t)(0.26f*sampleRate)+2);
    for (int side = 0; side < 2; ++side){
      for (int i = 0; i < 8; ++i){
        combMax[side][i] = (size_t)((COMB_TUNING[i] + side*STEREO_SPREAD)*scale);
        combs[side][i].line.init(combMax[side][i]);
        combs[side][i].length = combMax[side][i];
      }
      for (int i = 0; i < 4; ++i)
        allpasses[side][i].line.init((size_t)((ALLPASS_TUNING[i] + side*STEREO_SPREAD)*scale));
    }
  }

  void clear(){
    predelay.clear();
    echo.clear();
    for (int side = 0; side < 2; ++side){
      for (Comb& comb : combs[side])
        comb.clear();
      for (Allpass& allpass : allpasses[side])
        allpass.line.clear();
      hfStore[side] = 0;
    }
  }

  void set(const ReverbParams& p){
    float decay = std::clamp(p.decayTime, 0.1f, 20.0f);
    float hfRatio = std::clamp(p.decayHFRatio, 0.1f, 2.0f);
    //a lower density gives shorter combs (a smaller, more coloured room)
    float size = 0.4f + 0.6f*std::clamp(p.density, 0.0f, 1.0f);
    float hfCos = std::cos(2*PI*std::min(HF_REFERENCE, rate*0.49f)/rate);
    for (int side = 0; side < 2; ++side){
      for (int i = 0; i < 8; ++i){
        Comb& comb = combs[side][i];
        comb.length = std::max<size_t>((size_t)(combMax[side][i]*size), 1);
        if (comb.idx >= comb.length)
          comb.idx = 0;
        float seconds = comb.length/rate;
        //falls by 60 dB in decayTime, and at 5 kHz in decayTime*decayHFRatio
        comb.feedback = std::pow(10.0f, -3*seconds/decay);
        float hfGain = std::pow(comb.feedback, 1/hfRatio - 1);
        comb.damp = onePolePoleCos(hfGain, hfCos);
        //the same energy for every decay time (as EFX's density gain), over the eight combs
        comb.inGain = std::sqrt(1 - comb.feedback*comb.feedback)/std::sqrt(8.0f);
      }
      for (Allpass& allpass : allpasses[side])
        allpass.feedback = 0.15f + 0.35f*std::clamp(p.diffusion, 0.0f, 1.0f);
    }
    hfPole = onePolePole(std::clamp(p.gainHF, 0.0f, 1.0f), HF_REFERENCE, rate);
    float gain = std::clamp(p.gain, 0.0f, 1.0f);
    reflDelay = std::clamp(p.reflectionsDelay, 0.0f, 0.3f)*rate;
    lateDelay = std::clamp(p.lateReverbDelay, 0.0f, 0.1f)*rate;
    //two taps per side
    reflGain = gain*std::clamp(p.reflectionsGain, 0.0f, 3.16f)/std::sqrt(2.0f);
    lateGain = gain*std::clamp(p.lateReverbGain, 0.0f, 10.0f);
    float echoTime = std::clamp(p.echoTime, 0.075f, 0.25f);
    echoDelay = echoTime*rate;
    echoGain = std::clamp(p.echoDepth, 0.0f, 1.0f)*std::pow(10.0f, -3*echoTime/decay);
    float modTime = std::clamp(p.modulationTime, 0.04f, 4.0f);
    modStep = 1/(modTime*rate);
    //as OpenAL Soft: a swing of depth * 0.05 * time / 2 pi seconds
    modDepth = std::clamp(p.modulationDepth, 0.0f, 1.0f)*0.05f*modTime/(2*PI)*rate;
  }

  void process(const float* in, float* out, ma_uint32 frames){
    for (ma_uint32 f = 0; f < frames; ++f){
      predelay.write((in[2*f] + in[2*f+1])*0.5f);
      //early reflections: four taps, alternating between the sides
      float step = std::max(lateDelay*0.25f, rate*0.001f);
      float erL = predelay.tap(reflDelay+1) + predelay.tap(reflDelay+2*step);
      float erR = predelay.tap(reflDelay+step) - predelay.tap(reflDelay+3*step);
      //the late reverb, its start wobbling with the modulation
      float mod = modDepth > 0 ? modDepth*(1 + std::sin(2*PI*modPhase)) : 0;
      modPhase += modStep;
      if (modPhase >= 1)
        modPhase -= 1;
      float late = predelay.tap(reflDelay + lateDelay + mod + 1);
      if (echoGain > 0){
        late += echoGain*echo.tap(echoDelay);
        echo.write(late);
      }
      float wet[2];
      for (int side = 0; side < 2; ++side){
        float sum = 0;
        for (Comb& comb : combs[side])
          sum += comb.process(late);
        for (Allpass& allpass : allpasses[side])
          sum = allpass.process(sum);
        float er = side == 0 ? erL : erR;
        float x = sum*lateGain + er*reflGain;
        hfStore[side] = x*(1-hfPole) + hfStore[side]*hfPole;
        wet[side] = hfStore[side];
      }
      out[2*f] = wet[0];
      out[2*f+1] = wet[1];
    }
  }
};

struct Echo{
  float rate = 48000;
  DelayLine line;
  float tap1 = 1;
  float tap2 = 2;
  float damping = 0;
  float feedback = 0;
  float store = 0;
  float gains[2][2] = {{1, 0}, {0, 1}};

  void init(float sampleRate){
    rate = sampleRate;
    line.init((size_t)((0.207f+0.404f)*sampleRate)+4);
  }
  void clear(){line.clear(); store = 0;}
  void set(const EchoParams& p){
    tap1 = std::max(std::clamp(p.delay, 0.0f, 0.207f)*rate, 1.0f);
    tap2 = tap1 + std::clamp(p.lrDelay, 0.0f, 0.404f)*rate;
    damping = std::clamp(p.damping, 0.0f, 0.99f);
    feedback = std::clamp(p.feedback, 0.0f, 1.0f);
    float spread = std::clamp(p.spread, -1.0f, 1.0f);
    //the first tap to the left at -1, the second to the right
    gains[0][0] = std::sqrt((1-spread)*0.5f);
    gains[0][1] = std::sqrt((1+spread)*0.5f);
    gains[1][0] = gains[0][1];
    gains[1][1] = gains[0][0];
  }
  void process(const float* in, float* out, ma_uint32 frames){
    for (ma_uint32 f = 0; f < frames; ++f){
      float a = line.tap(tap1);
      float b = line.tap(tap2);
      store = b*(1-damping) + store*damping;
      line.write((in[2*f] + in[2*f+1])*0.5f + store*feedback);
      out[2*f] = a*gains[0][0] + b*gains[1][0];
      out[2*f+1] = a*gains[0][1] + b*gains[1][1];
    }
  }
};

struct Chorus{
  float rate = 48000;
  DelayLine lines[2];
  ChorusParams params;
  float phase = 0;

  void init(float sampleRate){
    rate = sampleRate;
    for (DelayLine& line : lines)
      line.init((size_t)(0.034f*sampleRate)+4);
  }
  void clear(){
    for (DelayLine& line : lines)
      line.clear();
  }
  void set(const ChorusParams& p){
    params = p;
    params.phase = std::clamp(p.phase, -180, 180);
    params.rate = std::clamp(p.rate, 0.0f, 10.0f);
    params.depth = std::clamp(p.depth, 0.0f, 1.0f);
    params.feedback = std::clamp(p.feedback, -0.99f, 0.99f);
    params.delay = std::clamp(p.delay, 0.0f, 0.016f);
  }
  //! the oscillator at a phase (0..1), -1..1
  float lfo(float ph) const{
    ph -= std::floor(ph);
    if (params.waveform == ChorusParams::SINE)
      return std::sin(2*PI*ph);
    return 4*std::fabs(ph-0.5f) - 1;
  }
  void process(const float* in, float* out, ma_uint32 frames){
    float delay = params.delay*rate;
    float step = params.rate/rate;
    float offset = params.phase/360.0f;
    for (ma_uint32 f = 0; f < frames; ++f){
      for (int side = 0; side < 2; ++side){
        float d = delay*(1 + params.depth*lfo(phase + side*offset));
        float y = lines[side].tap(d+1);
        lines[side].write(in[2*f+side] + y*params.feedback);
        out[2*f+side] = y;
      }
      phase += step;
      if (phase >= 1)
        phase -= 1;
    }
  }
};

//! a biquad in direct form 1
struct Biquad{
  float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
  float process(float x){
    float y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2;
    x2 = x1; x1 = x;
    y2 = y1; y1 = y;
    return y;
  }
  void clear(){x1 = x2 = y1 = y2 = 0;}
};

struct Distortion{
  float rate = 48000;
  float lpPole = 0;
  float lpStore[2] = {0, 0};
  Biquad bandpass[2];
  float k = 0;
  float gain = 0.05f;

  void init(float sampleRate){rate = sampleRate;}
  void clear(){
    lpStore[0] = lpStore[1] = 0;
    bandpass[0].clear();
    bandpass[1].clear();
  }
  void set(const DistortionParams& p){
    float edge = std::sin(std::clamp(p.edge, 0.0f, 1.0f)*PI*0.5f);
    edge = std::min(edge, 0.99f);
    k = 2*edge/(1-edge);
    gain = std::clamp(p.gain, 0.01f, 1.0f);
    float cutoff = std::clamp(p.lowpassCutoff, 80.0f, rate*0.49f);
    lpPole = std::exp(-2*PI*cutoff/rate);
    //an RBJ band-pass with OpenAL Soft's bandwidth in octaves
    float center = std::clamp(p.eqCenter, 80.0f, rate*0.45f);
    float octaves = std::clamp(p.eqBandwidth, 80.0f, 24000.0f)/(center*0.67f);
    float w0 = 2*PI*center/rate;
    float alpha = std::sin(w0)*std::sinh(std::log(2.0f)/2*octaves*w0/std::sin(w0));
    float a0 = 1+alpha;
    for (Biquad& bq : bandpass){
      bq.b0 = alpha/a0;
      bq.b1 = 0;
      bq.b2 = -alpha/a0;
      bq.a1 = -2*std::cos(w0)/a0;
      bq.a2 = (1-alpha)/a0;
    }
  }
  float shape(float x) const{return (1+k)*x/(1+k*std::fabs(x));}
  void process(const float* in, float* out, ma_uint32 frames){
    for (ma_uint32 f = 0; f < frames; ++f){
      for (int side = 0; side < 2; ++side){
        lpStore[side] = in[2*f+side]*(1-lpPole) + lpStore[side]*lpPole;
        //three steps of shaping, as OpenAL Soft (which also inverts the signal)
        float x = shape(lpStore[side]);
        x = -shape(x);
        x = shape(x);
        out[2*f+side] = bandpass[side].process(x)*gain;
      }
    }
  }
};

enum class Kind{
  None,
  Reverb,
  Echo,
  Chorus,
  Distortion,
};

struct Settings{
  Kind kind = Kind::None;
  float dry = 1;
  float wet = 1;
  ReverbParams reverb;
  EchoParams echo;
  ChorusParams chorus;
  DistortionParams distortion;
};

struct EffectNode;

}

struct AudioEffect::Impl{
  ~Impl();
  //! the node; its ma_node_base comes first
  std::unique_ptr<EffectNode> node;
  bool nodeInit = false;
  //! what the application set, for the audio thread
  Settings pending;
  ma_spinlock lock = 0;
  std::atomic<bool> changed{false};
};

namespace{

struct EffectNode{
  ma_node_base base;
  AudioEffect::Impl* owner = nullptr;
  //! what the audio thread runs
  Settings current;
  Reverb reverb;
  Echo echo;
  Chorus chorus;
  Distortion distortion;
  std::vector<float> wetBuffer;
  float dry = 1;
  float wet = 1;

  void apply(const Settings& settings){
    if (settings.kind != current.kind){
      //a new kind of effect starts without the old one's tail
      reverb.clear();
      echo.clear();
      chorus.clear();
      distortion.clear();
    }
    current = settings;
    switch (current.kind){
      case Kind::Reverb: reverb.set(current.reverb); break;
      case Kind::Echo: echo.set(current.echo); break;
      case Kind::Chorus: chorus.set(current.chorus); break;
      case Kind::Distortion: distortion.set(current.distortion); break;
      default: break;
    }
  }
};

void processEffect(ma_node* node, const float** in, ma_uint32* inCount, float** out, ma_uint32* outCount){
  EffectNode* effect = (EffectNode*)node;
  AudioEffect::Impl* owner = effect->owner;
  if (owner->changed.load(std::memory_order_acquire)){
    ma_spinlock_lock(&owner->lock);
    Settings settings = owner->pending;
    owner->changed = false;
    ma_spinlock_unlock(&owner->lock);
    effect->apply(settings);
  }
  const ma_uint32 frames = *outCount;
  const float* input = in[0];
  float* output = out[0];
  float* wet = effect->wetBuffer.data();
  const ma_uint32 chunk = (ma_uint32)(effect->wetBuffer.size()/2);
  //the gains change smoothly over the block
  float dry0 = effect->dry;
  float wet0 = effect->wet;
  float dry1 = effect->current.dry;
  float wet1 = effect->current.wet;
  for (ma_uint32 start = 0; start < frames; start += chunk){
    ma_uint32 count = std::min(chunk, frames-start);
    const float* src = input + 2*start;
    bool hasWet = true;
    switch (effect->current.kind){
      case Kind::Reverb: effect->reverb.process(src, wet, count); break;
      case Kind::Echo: effect->echo.process(src, wet, count); break;
      case Kind::Chorus: effect->chorus.process(src, wet, count); break;
      case Kind::Distortion: effect->distortion.process(src, wet, count); break;
      default: hasWet = false; break;
    }
    float* dst = output + 2*start;
    for (ma_uint32 f = 0; f < count; ++f){
      float t = frames > 1 ? (float)(start+f)/(frames-1) : 1;
      float d = dry0 + (dry1-dry0)*t;
      float w = wet0 + (wet1-wet0)*t;
      for (int c = 0; c < 2; ++c)
        dst[2*f+c] = src[2*f+c]*d + (hasWet ? wet[2*f+c]*w : 0);
    }
  }
  effect->dry = dry1;
  effect->wet = wet1;
  *inCount = frames;
  *outCount = frames;
}

ma_node_vtable effectVtable = {
  processEffect,
  nullptr,
  1,
  1,
  //the tails go on without input
  MA_NODE_FLAG_CONTINUOUS_PROCESSING
};

}

AudioEffect::Impl::~Impl(){
  if (nodeInit)
    ma_node_uninit(&node->base, nullptr);
}

AudioEffect::AudioEffect(std::unique_ptr<Impl> impl) : impl_(std::move(impl)){
}

AudioEffect::~AudioEffect(){
}

void AudioEffect::setNone(){
  ma_spinlock_lock(&impl_->lock);
  impl_->pending.kind = Kind::None;
  impl_->changed = true;
  ma_spinlock_unlock(&impl_->lock);
}

void AudioEffect::setReverb(const ReverbParams& params){
  ma_spinlock_lock(&impl_->lock);
  impl_->pending.kind = Kind::Reverb;
  impl_->pending.reverb = params;
  impl_->changed = true;
  ma_spinlock_unlock(&impl_->lock);
}

void AudioEffect::setEcho(const EchoParams& params){
  ma_spinlock_lock(&impl_->lock);
  impl_->pending.kind = Kind::Echo;
  impl_->pending.echo = params;
  impl_->changed = true;
  ma_spinlock_unlock(&impl_->lock);
}

void AudioEffect::setChorus(const ChorusParams& params){
  ma_spinlock_lock(&impl_->lock);
  impl_->pending.kind = Kind::Chorus;
  impl_->pending.chorus = params;
  impl_->changed = true;
  ma_spinlock_unlock(&impl_->lock);
}

void AudioEffect::setDistortion(const DistortionParams& params){
  ma_spinlock_lock(&impl_->lock);
  impl_->pending.kind = Kind::Distortion;
  impl_->pending.distortion = params;
  impl_->changed = true;
  ma_spinlock_unlock(&impl_->lock);
}

void AudioEffect::setMix(float dry, float wet){
  ma_spinlock_lock(&impl_->lock);
  impl_->pending.dry = std::max(dry, 0.0f);
  impl_->pending.wet = std::max(wet, 0.0f);
  impl_->changed = true;
  ma_spinlock_unlock(&impl_->lock);
}

ma_node* AudioInternal::effectNode(AudioEffect& effect){
  return &Access::impl(effect).node->base;
}

std::unique_ptr<AudioEffect> Audio::createEffect(AudioGroup group){
  if (!impl_->open)
    return nullptr;
  auto impl = std::make_unique<AudioEffect::Impl>();
  impl->node = std::make_unique<EffectNode>();
  EffectNode& node = *impl->node;
  node.owner = impl.get();
  float rate = (float)ma_engine_get_sample_rate(&impl_->engine);
  node.reverb.init(rate);
  node.echo.init(rate);
  node.chorus.init(rate);
  node.distortion.init(rate);
  //the effect's output for a part of a block
  node.wetBuffer.resize(1024*2);
  ma_uint32 channels = 2;
  ma_node_config config = ma_node_config_init();
  config.vtable = &effectVtable;
  config.pInputChannels = &channels;
  config.pOutputChannels = &channels;
  if (ma_node_init(ma_engine_get_node_graph(&impl_->engine), &config, nullptr, &node.base) != MA_SUCCESS)
    return nullptr;
  impl->nodeInit = true;
  ma_node_attach_output_bus(&node.base, 0, AudioInternal::groupNode(*impl_, group), 0);
  return std::unique_ptr<AudioEffect>(new AudioEffect(std::move(impl)));
}
