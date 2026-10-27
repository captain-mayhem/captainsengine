#ifndef CGE_AUDIOEFFECT_H
#define CGE_AUDIOEFFECT_H

#include <cge_export.h>

#include <memory>

namespace CGE{

namespace AudioInternal{
struct Access;
}

//! a reverb; the parameters and defaults of OpenAL's EAX reverb (EFX)
/*! Approximated with a Freeverb-style network (eight damped combs into
    four allpasses per side): the combs' feedback gives the decay time,
    their damping the decay at high frequencies (decayHFRatio), and their
    lengths shrink with the density. Early reflections are four taps after
    reflectionsDelay; the late reverb starts lateReverbDelay after them. The
    reverb's energy follows the gains as in EFX, independently of the decay
    time. The EFX parameters for low frequencies, panning, air absorption
    and roll-off have no counterpart. */
struct ReverbParams{
  //! 0..1: the modal density; lower gives shorter, more coloured combs
  float density = 1;
  //! 0..1: the echo density of the allpasses; lower sounds grainier
  float diffusion = 1;
  //! 0..1: the reverb's overall gain
  float gain = 0.32f;
  //! 0..1: its gain at 5 kHz
  float gainHF = 0.89f;
  //! 0.1..20 s: the time the late reverb takes to fall by 60 dB
  float decayTime = 1.49f;
  //! 0.1..2: the decay time at 5 kHz relative to decayTime
  float decayHFRatio = 0.83f;
  //! 0..3.16: the early reflections' gain
  float reflectionsGain = 0.05f;
  //! 0..0.3 s: the delay of the first reflection
  float reflectionsDelay = 0.007f;
  //! 0..10: the late reverb's gain
  float lateReverbGain = 1.26f;
  //! 0..0.1 s: the start of the late reverb after the first reflection
  float lateReverbDelay = 0.011f;
  //! 0.075..0.25 s and 0..1: a repeating echo in the late reverb
  float echoTime = 0.25f;
  float echoDepth = 0;
  //! 0.04..4 s and 0..1: a periodic pitch wobble of the reverb
  float modulationTime = 0.25f;
  float modulationDepth = 0;
};

//! an echo; the parameters and defaults of OpenAL's echo (EFX)
/*! Two taps of a delay line: the first after delay, the second lrDelay
    later, which also feeds back. */
struct EchoParams{
  //! 0..0.207 s: the first tap
  float delay = 0.1f;
  //! 0..0.404 s: the second tap after the first
  float lrDelay = 0.1f;
  //! 0..0.99: the high frequencies lost in each repetition
  float damping = 0.5f;
  //! 0..1: how much of the second tap is repeated
  float feedback = 0.5f;
  //! -1..1: the taps' panning (-1: the first left, the second right)
  float spread = -1;
};

//! a chorus or flanger; the parameters of OpenAL's chorus and flanger (EFX)
/*! A delay line per side whose delay a low-frequency oscillator moves
    around delay by depth (a factor), the right side phase degrees later.
    The defaults are the chorus's; flanger() gives the flanger's. */
struct ChorusParams{
  enum Waveform{
    SINE,
    TRIANGLE,
  };
  Waveform waveform = TRIANGLE;
  //! -180..180: the right oscillator's phase relative to the left
  int phase = 90;
  //! 0..10 Hz
  float rate = 1.1f;
  //! 0..1: the delay's swing as a factor of the delay
  float depth = 0.1f;
  //! -1..1: how much of the output goes back into the line
  float feedback = 0.25f;
  //! 0..0.016 s (a flanger's up to 0.004 s)
  float delay = 0.016f;

  //! OpenAL's flanger defaults
  static ChorusParams flanger(){
    ChorusParams params;
    params.waveform = TRIANGLE;
    params.phase = 0;
    params.rate = 0.27f;
    params.depth = 1;
    params.feedback = -0.5f;
    params.delay = 0.002f;
    return params;
  }
};

//! a distortion; the parameters and defaults of OpenAL's distortion (EFX)
/*! A low-pass, a waveshaper as hard as edge, a band-pass around eqCenter
    and the gain. */
struct DistortionParams{
  //! 0..1: the shaper's hardness
  float edge = 0.2f;
  //! 0.01..1: the output's gain
  float gain = 0.05f;
  //! 80..24000 Hz: the low-pass before the shaper
  float lowpassCutoff = 8000;
  //! 80..24000 Hz: the band-pass after it
  float eqCenter = 3600;
  float eqBandwidth = 3600;
};

//! An effect bus of the mixer (Audio::createEffect)
/*! Players and streams sent through it (setEffect) are mixed, and the mix
    goes on to the bus's group twice: unchanged with the dry gain and
    through the effect with the wet gain. It runs one effect at a time, or
    none (only the dry part, also with a wet gain). The effect's tail (a
    reverb's, an echo's) goes on after the sounds end, and stays when the
    effect changes to another of the same kind. The parameters can change
    at any time; they apply within the mixer's next block. Delete it after
    the players and streams sent through it, and before the Audio. */
class CGE_API AudioEffect{
public:
  ~AudioEffect();
  AudioEffect(const AudioEffect&) = delete;
  AudioEffect& operator=(const AudioEffect&) = delete;

  //! no effect: only the dry part
  void setNone();
  void setReverb(const ReverbParams& params);
  void setEcho(const EchoParams& params);
  void setChorus(const ChorusParams& params);
  void setDistortion(const DistortionParams& params);
  //! the gains of the unchanged and of the effect's part (1 and 1 to begin with)
  void setMix(float dry, float wet);

  struct Impl;
private:
  friend class Audio;
  friend struct AudioInternal::Access;
  explicit AudioEffect(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
};

}

#endif
