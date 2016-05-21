#include "PostProcessing.h"

#include <renderer/shader.h>

#include "Engine.h"
#include <system/allocation.h>

using namespace adv;

class Interpolator{
public:
  Interpolator() : mSource(0), mTarget(0), mCurrent(0), mTime(0), mTimeAccu(0){}
  Interpolator(float from, float to, float time) :
  mSource(from), mTarget(to), mCurrent(from), mTime(time), mTimeAccu(0){
  }
  void set(float from, float to, float time){
    mSource = from;
    mTarget = to;
    mCurrent = from;
    mTime = time;
    mTimeAccu = 0;
  }
  bool update(int time){
    if (mTimeAccu > mTime)
      return false;
    mTimeAccu += time;
    float alpha = mTimeAccu/mTime;
    if (alpha > 1.0f)
      alpha = 1.0f;
    mCurrent = mTarget*alpha+mSource*(1-alpha);
    return mTimeAccu < mTime;
  }
  float current(){
    return mCurrent;
  }
  float source() {return mSource;}
  float target() {return mTarget;}
  float time() {return mTime;}
  float currTime() {return (float)mTimeAccu;}
  void reset() {mCurrent = mSource; mTimeAccu = 0;}
  std::ostream& save(std::ostream& out){
    out << mSource << " " << mTarget << " " << mCurrent << " " << mTime << " " << mTimeAccu;
    return out;
  }
  std::istream& load(std::istream& in){
    in >> mSource >> mTarget >> mCurrent >> mTime >> mTimeAccu;
    return in;
  }
private:
  float mSource;
  float mTarget;
  float mCurrent;
  float mTime;
  int mTimeAccu;
};

PostProcessor::Effect::Effect(const char* vertexsource, const char* fragmentsource) : mShader(*CGE::Engine::instance()->getRenderer()->createShader()), mFade(false){
  mShader.addShader(CGE::Shader::VERTEX_SHADER, vertexsource);
  mShader.addShader(CGE::Shader::FRAGMENT_SHADER, fragmentsource);
  mShader.linkShaders();
}

PostProcessor::Effect::~Effect(){
  delete &mShader;
}

void PostProcessor::Effect::activate(bool fade, ...){
  std::list<Effect*>& activeEffects = Engine::instance()->getPostProcessor()->mActiveEffects;
  bool found = false;
  for (std::list<Effect*>::iterator iter = activeEffects.begin(); iter != activeEffects.end(); ++iter){
    if (*iter == this){
      found = true;
      break;
    }
  }
  if (!found){
    init(Vec2f((float)Engine::instance()->getResolution().x, (float)Engine::instance()->getResolution().y));
    Engine::instance()->getPostProcessor()->mActiveEffects.push_back(this);
  }
  mFade = fade;
}

void PostProcessor::Effect::deactivate(){
  deinit();
  Engine::instance()->getPostProcessor()->mActiveEffects.remove(this);
}

void PostProcessor::Effect::deactivate(int data){
  deactivate();
}

std::ostream& PostProcessor::Effect::save(std::ostream& out){
  out << mName << " " << mFade << " ";
  return out;
}

std::istream& PostProcessor::Effect::load(std::istream& in){
  in >> mFade;
  return in;
}

/*dark bloom*/

static const char bloomvs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 pos;\n"
"attribute vec4 texcoord;\n"
"\n"
"uniform vec2 tex_scale;\n"
"uniform vec2 pixel_offset;\n"
"\n"
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(pos.x*tex_scale.x, (0.0+pos.y)*tex_scale.y);\n"
"  tex_coord2 = vec2(pos.x*2.0-1.0, (0.0+pos.y)*2.0-1.0)*pixel_offset*2.0;\n"
"  gl_Position = vec4(pos.x*2.0-1.0, pos.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
""
"@HLSL"
""
"cbuffer perObject{\n"
"  float2 tex_scale;\n"
"  float2 pixel_offset;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
"  float2 texcoord: TEXCOORD0;\n"
"  float3 normal: NORMAL;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"  float2 tcoord2 : TEXCOORD1;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  outp.tcoord = float2(inp.pos.x*tex_scale.x, (1.0-inp.pos.y)*tex_scale.y);\n"
"  outp.tcoord2 = float2(inp.pos.x*2.0-1.0, (1.0-inp.pos.y)*2.0-1.0)*pixel_offset*2.0;\n"
"  outp.vPos = float4(inp.pos.x*2.0-1.0, inp.pos.y*2.0-1.0, 0.0, 1.0);\n"
"  return outp;\n"
"}\n"
"\n"
;

static const char darkbloomfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform float strength;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
//"  color.rgb = color.rgb*color.rgb*vec3(2.0);\n"
"  vec4 intensity = vec4(strength)/vec4(6.0);\n"
"  intensity.a = 0.0;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st)*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(2.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(3.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(4.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(5.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(6.0))*intensity;\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float strength;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"  float2 tex_coord2 : TEXCOORD1;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  float4 intensity = float4(strength, strength, strength, strength)/float4(6.0, 6.0, 6.0, 6.0);\n"
"  intensity.a = 0.0;\n"
"  color = color-tex.Sample(sampl, inp.tex_coord-inp.tex_coord2)*intensity;\n"
"  color = color-tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(2.0, 2.0))*intensity;\n"
"  color = color-tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(3.0, 3.0))*intensity;\n"
"  color = color-tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(4.0, 4.0))*intensity;\n"
"  color = color-tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(5.0, 5.0))*intensity;\n"
"  color = color-tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(6.0, 6.0))*intensity;\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

static const char hellfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform float strength;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  vec3 intensity = vec3(1.0)-vec3(strength);\n"
"  color.rgb = color.rgb*texture2D(texture, tex_coord.st-tex_coord2.st).rgb/**intensity*/;\n"
"  color.rgb = color.rgb*texture2D(texture, tex_coord.st-tex_coord2.st*vec2(2.0)).rgb/**intensity*/;\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float strength;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"  float2 tex_coord2 : TEXCOORD1;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  color.rgb = color.rgb*tex.Sample(sampl, inp.tex_coord-inp.tex_coord2).rgb;\n"
"  color.rgb = color.rgb*tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(2.0, 2.0)).rgb;\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

static const char whooshfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform float strength;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  float intensity = strength/6.0;\n"
"  color = mix(color, texture2D(texture, tex_coord.st-tex_coord2.st), intensity);\n"
"  color = mix(color, texture2D(texture, tex_coord.st-tex_coord2.st*vec2(2.0)), intensity);\n"
"  color = mix(color, texture2D(texture, tex_coord.st-tex_coord2.st*vec2(3.0)), intensity);\n"
"  color = mix(color, texture2D(texture, tex_coord.st-tex_coord2.st*vec2(4.0)), intensity);\n"
"  color = mix(color, texture2D(texture, tex_coord.st-tex_coord2.st*vec2(5.0)), intensity);\n"
"  color = mix(color, texture2D(texture, tex_coord.st-tex_coord2.st*vec2(6.0)), intensity);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float strength;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"  float2 tex_coord2 : TEXCOORD1;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  float4 intensity = float4(strength, strength, strength, strength)/float4(6.0, 6.0, 6.0, 6.0);\n"
"  intensity.a = 0.0;\n"
"  color = lerp(color, tex.Sample(sampl, inp.tex_coord-inp.tex_coord2), intensity);\n"
"  color = lerp(color, tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(2.0, 2.0)), intensity);\n"
"  color = lerp(color, tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(3.0, 3.0)), intensity);\n"
"  color = lerp(color, tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(4.0, 4.0)), intensity);\n"
"  color = lerp(color, tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(5.0, 5.0)), intensity);\n"
"  color = lerp(color, tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(6.0, 6.0)), intensity);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

static const char bloomfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"uniform sampler2D tex;\n"
"uniform float strength;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(tex, tex_coord.st);\n"
//"  color.rgb = color.rgb*color.rgb*vec3(2.0);\n"
"  vec4 intensity = vec4(strength)/vec4(6.0);\n"
"  intensity.a = 0.0;\n"
"  color = color+texture2D(tex, tex_coord.st-tex_coord2.st)*intensity;\n"
"  color = color+texture2D(tex, tex_coord.st-tex_coord2.st*vec2(2.0))*intensity;\n"
"  color = color+texture2D(tex, tex_coord.st-tex_coord2.st*vec2(3.0))*intensity;\n"
"  color = color+texture2D(tex, tex_coord.st-tex_coord2.st*vec2(4.0))*intensity;\n"
"  color = color+texture2D(tex, tex_coord.st-tex_coord2.st*vec2(5.0))*intensity;\n"
"  color = color+texture2D(tex, tex_coord.st-tex_coord2.st*vec2(6.0))*intensity;\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float strength;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"  float2 tex_coord2 : TEXCOORD1;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  float4 intensity = float4(strength, strength, strength, strength)/float4(6.0, 6.0, 6.0, 6.0);\n"
"  intensity.a = 0.0;\n"
"  color = color+tex.Sample(sampl, inp.tex_coord-inp.tex_coord2)*intensity;\n"
"  color = color+tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(2.0, 2.0))*intensity;\n"
"  color = color+tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(3.0, 3.0))*intensity;\n"
"  color = color+tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(4.0, 4.0))*intensity;\n"
"  color = color+tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(5.0, 5.0))*intensity;\n"
"  color = color+tex.Sample(sampl, inp.tex_coord-inp.tex_coord2*float2(6.0, 6.0))*intensity;\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";
;

class BloomEffect : public PostProcessor::Effect{
public:
  BloomEffect(const char* name, const char* fragmentshader) : Effect(bloomvs, fragmentshader){
    mName = name;
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "tex");
    mShader.uniform(tex, 0);
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    mShader.uniform(scale, size.x/powx, size.y/powy);
    int pixeloffset = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "pixel_offset");
    mShader.uniform(pixeloffset, 1.0f/size.x, 1.0f/size.y);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "strength");
    mShader.deactivate();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args,double);
    mAnimate = va_arg(args,int) != 0;
    va_end(args);
    if (fade){
      mInterpolator.set(0, strength, 2000);
      mState = FADEIN;
    }
    else{
      mInterpolator.set(strength, fade ? strength/2 : strength, 1000);
      mState = ANIMATE;
    }
    Effect::activate(fade);
  }
  virtual void deactivate(){
    if (mFade){
      mInterpolator.set(mInterpolator.current(), 0, mInterpolator.currTime()*2);
      mState = FADEOUT;
    }
    else
      Effect::deactivate();
  }
  virtual bool update(unsigned time) {
    if (!mInterpolator.update(time)){
      if (mState == FADEIN){
        mInterpolator.set(mInterpolator.current(), mInterpolator.current()/2, mInterpolator.time());
        mInterpolator.reset();
        mState = ANIMATE;
      }
      else if (mState == FADEOUT){
        Effect::deactivate();
        return false;
      }
      else if (mAnimate){
        mInterpolator.set(mInterpolator.current(), mInterpolator.source(), mInterpolator.time());
        mInterpolator.reset();
      }
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mAnimate << " " << mState << " ";
    mInterpolator.save(out);
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mAnimate;
    int tmp;
    in >> tmp;
    mState = (State)tmp;
    mInterpolator.load(in);
    return in;
  }
private:
  enum State{
    FADEIN,
    ANIMATE,
    FADEOUT
  };
  bool mAnimate;
  int mIntensityLoc;
  Interpolator mInterpolator;
  State mState;
};

/*noise*/

static const char stdvs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 pos;\n"
"attribute vec4 texcoord;\n"
"\n"
"uniform vec2 tex_scale;\n"
"\n"
"varying vec2 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(pos.x*tex_scale.x, (0.0+pos.y)*tex_scale.y);\n"
"  gl_Position = vec4(pos.x*2.0-1.0, pos.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
""
"@HLSL"
""
"cbuffer perObject{\n"
"  float2 tex_scale;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
"  float2 texcoord: TEXCOORD0;\n"
"  float3 normal: NORMAL;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  outp.tcoord = float2(inp.pos.x*tex_scale.x, (1.0-inp.pos.y)*tex_scale.y);\n"
"  outp.vPos = float4(inp.pos.x*2.0-1.0, inp.pos.y*2.0-1.0, 0.0, 1.0);\n"
"  return outp;\n"
"}\n"
"\n";

static const char stdfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

static const char noisefs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform sampler2D blendtex;\n"
"uniform float opacity;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  vec4 blendcol = vec4(1.0);\n"
"  blendcol.rgb = texture2D(blendtex, tex_coord.st).rrr;\n"
"  color = mix(color, blendcol, opacity);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex : register(t0);\n"
"SamplerState sampl : register(s0);\n"
"Texture2D blendtex : register(t1);\n"
"SamplerState blendsampl : register(s1);\n"
"\n"
"cbuffer perDraw{\n"
"  float opacity;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  float4 blendcol = float4(1.0, 1.0, 1.0, 1.0);\n"
"  blendcol.rgb = blendtex.Sample(blendsampl, inp.tex_coord).rrr;\n"
"  color = lerp(color, blendcol, opacity);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

static const char clrnoisefs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform sampler2D blendtex;\n"
"uniform float opacity;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  vec4 blendcol = vec4(1.0);\n"
"  blendcol.rgb = texture2D(blendtex, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, opacity);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex : register(t0);\n"
"SamplerState sampl : register(s0);\n"
"Texture2D blendtex : register(t1);\n"
"SamplerState blendsampl : register(s1);\n"
"\n"
"cbuffer perDraw{\n"
"  float opacity;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  float4 blendcol = float4(1.0, 1.0, 1.0, 1.0);\n"
"  blendcol.rgb = blendtex.Sample(blendsampl, inp.tex_coord).rgb;\n"
"  color = lerp(color, blendcol, opacity);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

class NoiseEffect : public PostProcessor::Effect{
public:
  NoiseEffect(const char* name, const char* fragmentshader, int numChannels) : Effect(stdvs, fragmentshader), mBlendTex(0), mChannels(numChannels){
    mName = name;
  }
  virtual void init(const Vec2f& size){
    mImage.setFormat(mChannels, (int)size.x/2, (int)size.y/2);
    mImage.allocateData();
    for (unsigned i = 0; i < mImage.getImageSize(); ++i){
      mImage.getData()[i] = (unsigned char)((rand()/(float)RAND_MAX)*255);
    }
    Vec2i imgsize;
    Vec2f imgscale;
    mBlendTex = Engine::instance()->genTexture(&mImage, imgsize, imgscale);
    mBlendTex->activate(1);
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    int blendtex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendtex");
    mShader.uniform(blendtex, 1);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "opacity");
    mShader.deactivate();
  }
  virtual void deinit(){
    if (mBlendTex != 0){
      delete mBlendTex;
      mBlendTex = NULL;
    }
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args,double);
    va_end(args);
    if (fade){
      mInterpolator.set(0, strength, 1000);
    }
    else{
      mInterpolator.set(strength, strength, 1000);
    }
    mFadeout = false;
    Effect::activate(fade);
  }
  virtual void deactivate(){
    if (mFade){
      mInterpolator.set(mInterpolator.current(), 0, mInterpolator.currTime());
      mFadeout = true;
    }
    else
      Effect::deactivate();
  }
  virtual bool update(unsigned time) {
    if (!mInterpolator.update(time)){
      if (mFadeout){
        Effect::deactivate();
        return false;
      }
    }
    for (unsigned i = 0; i < mImage.getImageSize(); ++i){
      mImage.getData()[i] = (unsigned char)((rand()/(float)RAND_MAX)*255);
    }
    mBlendTex->activate(1);
    mBlendTex->update(0, 0, mImage.getWidth(), mImage.getHeight(), mImage.getData());
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " ";
    mInterpolator.save(out);
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mFadeout;
    mInterpolator.load(in);
    return in;
  }
private:
  int mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
  CGE::Texture* mBlendTex;
  CGE::Image mImage;
  int mChannels;
};

static const char motionblurfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform sampler2D motion1;\n"
"uniform sampler2D motion2;\n"
"uniform sampler2D motion3;\n"
//"uniform sampler2D motion4;\n"
//"uniform sampler2D motion5;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  vec4 blendcol = vec4(1.0);\n"
"  color.rgb = texture2D(motion3, tex_coord.st).rgb;\n"
"  blendcol.rgb = texture2D(motion2, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, 0.5);\n"
"  blendcol.rgb = texture2D(motion1, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, 0.5);\n"
"  blendcol.rgb = texture2D(texture, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, 0.5);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex : register(t0);\n"
"SamplerState sampl : register(s0);\n"
"Texture2D motion1 : register(t1);\n"
"SamplerState motion1samp : register(s1);\n"
"Texture2D motion2 : register(t2);\n"
"SamplerState motion2samp : register(s2);\n"
"Texture2D motion3 : register(t3);\n"
"SamplerState motion3samp : register(s3);\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  float4 blendcol = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color.rgb = motion3.Sample(motion3samp, inp.tex_coord).rgb;\n"
"  blendcol.rgb = motion2.Sample(motion2samp, inp.tex_coord).rgb;\n"
"  color = lerp(color, blendcol, 0.5);\n"
"  blendcol.rgb = motion1.Sample(motion1samp, inp.tex_coord).rgb;\n"
"  color = lerp(color, blendcol, 0.5);\n"
"  blendcol.rgb = tex.Sample(sampl, inp.tex_coord).rgb;\n"
"  color = lerp(color, blendcol, 0.5);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

class MotionBlurEffect : public PostProcessor::Effect{
public:
  MotionBlurEffect(const char* name, const char* fragmentshader) : Effect(stdvs, motionblurfs), mStdShader(*CGE::Engine::instance()->getRenderer()->createShader()){
    mName = name;
    mStdShader.addShader(CGE::Shader::VERTEX_SHADER, stdvs);
    mStdShader.addShader(CGE::Shader::FRAGMENT_SHADER, stdfs);
    mStdShader.linkShaders();
  }
  ~MotionBlurEffect(){
    for (std::list<RenderableBlitObject*>::iterator iter = mPrevFrames.begin(); iter != mPrevFrames.end(); ++iter){
      delete *iter;
    }
    for (std::list<RenderableBlitObject*>::iterator iter = mGenFrames.begin(); iter != mGenFrames.end(); ++iter){
      delete *iter;
    }
    mPrevFrames.clear();
    mGenFrames.clear();
    delete &mStdShader;
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    int motion = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "motion1");
    mShader.uniform(motion, 1);
    motion = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "motion2");
    mShader.uniform(motion, 2);
    motion = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "motion3");
    mShader.uniform(motion, 3);
    mShader.deactivate();
    mStdShader.activate();
    tex = mStdShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mStdShader.uniform(tex, 0);
    scale = mStdShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    mStdShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mStdShader.uniform(scale, size.x/powx, size.y/powy);
    mStdShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mStdShader.deactivate();
    for (int i = 0; i < 3; ++i){
      RenderableBlitObject* rbo = new RenderableBlitObject(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, 0);
      rbo->realize();
      mGenFrames.push_back(rbo);
    }
  }
  virtual void deinit(){
    for (std::list<RenderableBlitObject*>::iterator iter = mPrevFrames.begin(); iter != mPrevFrames.end(); ++iter){
      mGenFrames.push_back(*iter);
    }
    mPrevFrames.clear();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args,double);
    va_end(args);
    mTakeFrame = (int)(strength*5);
    mTakeCount = mTakeFrame;
    Effect::activate(fade);
  }
  virtual void deactivate(){
    Effect::deactivate();
  }
  virtual bool update(unsigned time) {
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    if (mTakeCount >= mTakeFrame){
      if (mPrevFrames.size() < 3){
        RenderableBlitObject* rbo = mGenFrames.front();
        mGenFrames.pop_front();
        mPrevFrames.push_front(rbo);
      }
      else{
        RenderableBlitObject* rbo = mPrevFrames.back();
        mPrevFrames.pop_back();
        mPrevFrames.push_front(rbo);
      }
      mTakeCount = -1;
      mStdShader.activate();
      mPrevFrames.front()->bind();
      CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
      Engine::instance()->drawQuad();
      mPrevFrames.front()->unbind();
      mStdShader.deactivate();
    }
    mTakeCount++;

    if (mPrevFrames.size() < 3)
      mStdShader.activate();
    else
      mShader.activate();
    int count = 0;
    for (std::list<RenderableBlitObject*>::iterator iter = mPrevFrames.begin(); iter != mPrevFrames.end(); ++iter){
      (*iter)->getTexture()->activate(1 + count);
      ++count;
    }
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    Engine::instance()->drawQuad();
    count = 0;
    for (std::list<RenderableBlitObject*>::iterator iter = mPrevFrames.begin(); iter != mPrevFrames.end(); ++iter){
      (*iter)->getTexture()->deactivate(1 + count);
      ++count;
    }
    if (mPrevFrames.size() < 3)
      mStdShader.deactivate();
    else
      mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mTakeFrame;
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mTakeFrame;
    mTakeCount = mTakeFrame;
    return in;
  }
private:
  CGE::Shader& mStdShader;
  std::list<RenderableBlitObject*> mPrevFrames;
  std::list<RenderableBlitObject*> mGenFrames;
  int mTakeFrame;
  int mTakeCount;
};

static const char heatfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform sampler2D blendtex;\n"
"uniform vec2 tex_scale;\n"
"uniform vec2 pixel_offset;\n"
"\n"
"void main(){\n"
"  float xoffset = texture2D(blendtex, vec2(0.5, tex_coord.t)).r*2.0-1.0;\n"
"  vec4 color = vec4(1.0);\n"
"  float xcoord = clamp(tex_coord.s+xoffset*pixel_offset.x, 0.0, tex_scale.x-pixel_offset.x/2.0);\n"
"  color = texture2D(texture, vec2(xcoord,tex_coord.t));\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex : register(t0);\n"
"SamplerState sampl : register(s0);\n"
"Texture2D blendtex : register(t1);\n"
"SamplerState blendsampl : register(s1);\n"
"\n"
"cbuffer perDraw{\n"
"  float2 tex_scale;\n"
"  float2 pixel_offset;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float xoffset = blendtex.Sample(blendsampl, float2(0.5, inp.tex_coord.y)).r*2.0-1.0;\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  float xcoord = clamp(inp.tex_coord.x+xoffset*pixel_offset.x, 0.0, tex_scale.x-pixel_offset.x/2.0);\n"
"  color = tex.Sample(sampl, float2(xcoord, inp.tex_coord.y));\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

class HeatEffect : public PostProcessor::Effect{
public:
  enum Type{
    RANDOM,
    SINE,
  };
  HeatEffect(const std::string& name, Type type) : Effect(stdvs, heatfs), mBlendTex(0), mTimeAccu(0), mLineCount(0){
    mName = name;
    mType = type;
  }
  virtual void init(const Vec2f& size){
    mImage.setFormat(1, 1, (int)size.y/2);
    mImage.allocateData();
    for (unsigned i = 0; i < mImage.getImageSize(); ++i){
      if (mType == RANDOM)
        mImage.getData()[i] = (unsigned char)((rand()/(float)RAND_MAX)*255);
      else if (mType == SINE)
        mImage.getData()[i] = (unsigned char)(sin((float)mLineCount++/6.0f)*127+127);
    }
    Vec2i imgsize;
    Vec2f imgscale;
    mBlendTex = Engine::instance()->genTexture(&mImage, imgsize, imgscale);
    mBlendTex->activate(1);
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    int blendtex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendtex");
    mShader.uniform(blendtex, 1);
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    int vscale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(vscale, size.x / powx, size.y / powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    int scale = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "tex_scale");
    int pixeloffset = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "pixel_offset");
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.uniform(pixeloffset, 1.0f/size.x, 1.0f/size.y);
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.deactivate();
  }
  virtual void deinit(){
    if (mBlendTex != 0){
      delete mBlendTex;
      mBlendTex = NULL;
    }
  }
  virtual void activate(bool fade, ...){
    Effect::activate(fade);
    if (fade){
      memset(mImage.getData(), 127, mImage.getImageSize());
      mBlendTex->activate(1);
      mBlendTex->update(0, 0, mImage.getWidth(), mImage.getHeight(), mImage.getData());
    }
    mFadeout = false;
  }
  virtual void deactivate(){
    if (mFade){
      mFadeout = true;
      mFadeoutPixels = mImage.getHeight();
    }
    else
      Effect::deactivate();
  }
  virtual bool update(unsigned time) {
    if (mFadeout && mFadeoutPixels <= 0){
      Effect::deactivate();
      return false;
    }
    mTimeAccu += time;
    if (mTimeAccu > 50){
      time = 1;
      memmove(mImage.getData(), mImage.getData()+time, mImage.getImageSize()-time);
      for (unsigned i = mImage.getImageSize()-time; i < mImage.getImageSize(); ++i){
        if (mType == RANDOM)
          mImage.getData()[i] = mFadeout ? 127 : (unsigned char)((rand()/(float)RAND_MAX)*255);
        else if (mType == SINE)
          mImage.getData()[i] = (unsigned char)(sin((float)mLineCount++/6.0f)*127+127);
      }
      mBlendTex->activate(1);
      mBlendTex->update(0, 0, mImage.getWidth(), mImage.getHeight(), mImage.getData());
      mTimeAccu -= 50;
      if (mFadeout)
        mFadeoutPixels -= time;
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " " << mTimeAccu << " " << mFadeoutPixels << " " << mType;
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    int tmp;
    in >> mFadeout >> mTimeAccu >> mFadeoutPixels >> tmp;
    mType = (Type)mType;
    return in;
  }
private:
  bool mFadeout;
  CGE::Texture* mBlendTex;
  CGE::Image mImage;
  int mTimeAccu;
  int mFadeoutPixels;
  int mLineCount;
  Type mType;
};

static const char druggedvs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 pos;\n"
"attribute vec2 texcoord;\n"
"attribute vec2 texcoord2; /*offset*/\n"
"\n"
"uniform vec2 tex_scale;\n"
"\n"
"varying vec2 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(pos.x*tex_scale.x, (0.0+pos.y)*tex_scale.y);\n"
"  gl_Position = vec4(pos.x*2.0-1.0+texcoord2.x, pos.y*2.0-1.0+texcoord2.y, 0.0, 1.0);\n"
"}\n"
""
"@HLSL"
""
"cbuffer perObject{\n"
"  float2 tex_scale;\n"
"}\n"
"\n"
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
"  float2 texcoord: TEXCOORD0;\n"
"  float3 texcoord2: TEXCOORD1;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tcoord : TEXCOORD0;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  outp.tcoord = float2(inp.pos.x*tex_scale.x, (1.0-inp.pos.y)*tex_scale.y);\n"
"  outp.vPos = float4(inp.pos.x*2.0-1.0+inp.texcoord2.x, inp.pos.y*2.0-1.0+inp.texcoord2.y, 0.0, 1.0);\n"
"  return outp;\n"
"}\n"
"\n";

class DruggedEffect : public PostProcessor::Effect{
public:
  DruggedEffect() : Effect(druggedvs, stdfs), mVB(NULL){
    mName = "drugged";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mVB = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
    mVB->create(VB_POSITION | VB_TEXCOORD | VB_TEXCOORD2, 4);
    mShader.activate();
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.deactivate();
  }
  virtual void deinit(){
    delete mVB;
    mVB = NULL;
    Effect::deinit();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args,double);
    mStrength = strength*0.25f;
    va_end(args);
    Effect::activate(fade);
    mFadeout = false;
    for (int i = 0; i < 8; ++i){
      float time;
      float tmp = newPos(time);
      mVerts[i].set(0, tmp, 1000);
    }
  }
  virtual void deactivate(){
     mFadeout = true;
     for (int i = 0; i < 8; ++i){
       mVerts[i].set(mVerts[i].current(), 0, 1000);
     }
  }
  virtual bool update(unsigned time) {
    int finishCount = 0;
    for (int i = 0; i < 8; ++i){
      if (!mVerts[i].update(time)){
        if (mFadeout)
          ++finishCount;
        else{
          float time;
          float tmp = newPos(time);
          mVerts[i].set(mVerts[i].current(), tmp, time);
        }
      }
    }
    if (finishCount >= 8){
      Effect::deactivate();
      return false;
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    float tmp[8];
    for (int i = 0; i < 8; ++i){
      tmp[i] = mVerts[i].current();
      if (i == 0 || i == 2 || i == 3 || i == 7)
        tmp[i] *= -1;
    }
    mVB->lockVertexPointer();
    Engine::instance()->fillWithStdQuad(mVB);
    mVB->setTexCoord2(0, CGE::Vec2f(tmp[0], tmp[1]));
    mVB->setTexCoord2(1, CGE::Vec2f(tmp[2], tmp[3]));
    mVB->setTexCoord2(2, CGE::Vec2f(tmp[4], tmp[5]));
    mVB->setTexCoord2(3, CGE::Vec2f(tmp[6], tmp[7]));
    mVB->unlockVertexPointer();
    mVB->activate();
    mVB->draw(CGE::VB_Tristrip, NULL);
    mShader.deactivate();
    Engine::instance()->restoreRenderDefaults();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " " << mStrength;
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mFadeout >> mStrength;
    if (mFadeout)
      for (int i = 0; i < 8; ++i){
        float time;
        float tmp = newPos(time);
        mVerts[i].set(tmp, 0, 1000);
      }
    else
      for (int i = 0; i < 8; ++i){
        float time;
        float tmp = newPos(time);
        float tmp2 = newPos(time);
        mVerts[i].set(tmp, tmp2, time);
      }
    return in;
  }
private:
  float newPos(float& time){
    time = (rand()/(float)RAND_MAX)*1000+100;
    return ((rand()/(float)RAND_MAX)*2-0.5f)*mStrength;
  }
  bool mFadeout;
  Interpolator mVerts[8];
  CGE::VertexBuffer* mVB;
  float mStrength;
};

static const char lightningfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform sampler2D blendtex;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  vec4 blendcol = vec4(1.0);\n"
"  blendcol = texture2D(blendtex, tex_coord.st);\n"
"  color = mix(color, blendcol, blendcol.a);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex : register(t0);\n"
"SamplerState sampl : register(s0);\n"
"Texture2D blendtex : register(t1);\n"
"SamplerState blendsampl : register(s1);\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  float4 blendcol = float4(1.0, 1.0, 1.0, 1.0);\n"
"  blendcol = blendtex.Sample(blendsampl, inp.tex_coord);\n"
"  color = lerp(color, blendcol, blendcol.a);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

static const char drawvs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"\n"
"varying vec4 vert_color;\n"
"\n"
"void main(){\n"
"  vert_color = color;\n"
"  gl_Position = vec4(pos.x*2.0-1.0, pos.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
""
"@HLSL"
""
"struct VSInput{\n"
"  float3 pos : POSITION;\n"
"  float4 color : COLOR0;\n"
"};\n"
"\n"
"struct VSOutput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float4 vert_color : COLOR0;\n"
"};\n"
"\n"
"VSOutput main(VSInput inp){\n"
"  VSOutput outp;\n"
"  outp.vert_color = inp.color;\n"
"  outp.vPos = float4(inp.pos.x*2.0-1.0, (1.0-inp.pos.y)*2.0-1.0, 0.0, 1.0);\n"
"  return outp;\n"
"}\n"
"\n";

static const char drawfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec4 vert_color;\n"
"\n"
"void main(){\n"
"  vec4 color = vert_color;\n"
"  gl_FragColor = color;\n"
//"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float4 vert_color : COLOR0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = inp.vert_color;\n"
"  return color;\n"
"}\n"
"\n";

class LightningEffect : public PostProcessor::Effect{
public:
  LightningEffect() : Effect(stdvs, lightningfs), mFBO(NULL), mDrawShader(*CGE::Engine::instance()->getRenderer()->createShader()){
    mName = "lightning";
    mDrawShader.addShader(CGE::Shader::VERTEX_SHADER, drawvs);
    mDrawShader.addShader(CGE::Shader::FRAGMENT_SHADER, drawfs);
    mDrawShader.linkShaders();
  }
  ~LightningEffect(){
    deactivate();
    delete &mDrawShader;
  }
  virtual void init(const Vec2f& size){
    mFBO = new RenderableBlitObject((int)size.x, (int)size.y, 0);
    mFBO->realize();
    Vec2i imgsize;
    Vec2f imgscale;
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    int blendtex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendtex");
    mShader.uniform(blendtex, 1);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.deactivate();
  }
  virtual void deinit(){
    delete mFBO;
    mFBO = NULL;
  }
  virtual void activate(bool fade, ...){
    Lightning* lightning = new Lightning;
    Lightning& ltn = *lightning;
    va_list args;
    va_start(args, fade);
    int slot = va_arg(args,int);
    ltn.start.x = (float)va_arg(args, double);
    ltn.start.y = (float)va_arg(args, double);
    ltn.end.x = (float)va_arg(args, double);
    ltn.end.y = (float)va_arg(args, double);
    ltn.color.r = (unsigned char)va_arg(args, int);
    ltn.color.g = (unsigned char)va_arg(args, int);
    ltn.color.b = (unsigned char)va_arg(args, int);
    ltn.numSpikes = va_arg(args, int);
    ltn.height = (float)va_arg(args, double);
    ltn.delay = va_arg(args, int);
    va_end(args);
    ltn.timeaccu = ltn.delay;
    ltn.verts = NULL;
    ltn.timeaccu2 = (int)(ltn.delay*1.5f);
    ltn.verts2 = NULL;
    delete mLigthnings[slot];
    mLigthnings[slot] = lightning;
    Effect::activate(fade);
  }
  virtual void deactivate(){
    for (std::map<int,Lightning*>::iterator iter = mLigthnings.begin(); iter != mLigthnings.end(); ++iter){
      delete iter->second;
    }
    mLigthnings.clear();
    Effect::deactivate();
  }
  virtual void deactivate(int data){
    std::map<int,Lightning*>::iterator iter = mLigthnings.find(data);
    if (iter != mLigthnings.end()){
      Lightning* lt = iter->second;
      mLigthnings.erase(iter);
      delete lt;
    }
    if (mLigthnings.empty())
      Effect::deactivate();
  }
  virtual bool update(unsigned time) {   
    mFBO->bind();
    CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
    rend->setClearColor(CGE::Vec4f(0.0, 0.0, 0.0, 0.0));
    rend->clear(COLORBUFFER | ZBUFFER);
    mDrawShader.activate();
    for (std::map<int,Lightning*>::iterator iter = mLigthnings.begin(); iter != mLigthnings.end(); ++iter){
      iter->second->timeaccu += time;
      iter->second->timeaccu2 += time;
      bool renderNew = false;
      bool renderNew2 = false;
      if (iter->second->timeaccu >= iter->second->delay){
        iter->second->timeaccu -= iter->second->delay;
        renderNew = true;
      }
      if (iter->second->timeaccu2 >= iter->second->delay){
        iter->second->timeaccu2 -= iter->second->delay;
        renderNew2 = true;
      }
      int numVerts = iter->second->numSpikes+2;
      if (renderNew){
        renderNew = false;
        drawFlash(numVerts, iter->second, false);
      }
      if (renderNew2){
        renderNew2 = false;
        drawFlash(numVerts, iter->second, true);
      }
      iter->second->verts->activate();
      iter->second->verts->draw(CGE::VB_Linestrip, NULL);
      iter->second->verts2->activate();
      iter->second->verts2->draw(CGE::VB_Linestrip, NULL);
    }
    mDrawShader.deactivate();
    Engine::instance()->restoreRenderDefaults();
    mFBO->unbind();
    
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    mFBO->getTexture()->activate(1);
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    //mShader.uniform(mIntensityLoc, mInterpolator.current());
    Engine::instance()->drawQuad();
    mFBO->getTexture()->deactivate(1);
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mLigthnings.size() << "\n";
    for (std::map<int,Lightning*>::iterator iter = mLigthnings.begin(); iter != mLigthnings.end(); ++iter){
      out << iter->first << " ";
      iter->second->save(out);
      out << "\n";
    }
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    int numFlashes;
    in >> numFlashes;
    for (int i = 0; i < numFlashes; ++i){
      int num;
      in >> num;
      Lightning* ltn = new Lightning;
      ltn->load(in);
      mLigthnings[num] = ltn;
    }
    return in;
  }
private:
  struct Lightning{
    Vec2f start;
    Vec2f end;
    Color color;
    int numSpikes;
    float height;
    int delay;

    int timeaccu;
    CGE::VertexBuffer* verts;
    int timeaccu2;
    CGE::VertexBuffer* verts2;

    ~Lightning(){
      delete verts;
      delete verts2;
    }

    std::ostream& save(std::ostream& out){
      out << start.x << " " << start.y << " " << end.x << " " << end.y << " ";
      out << color << " ";
      out << numSpikes << " " << height << " " << delay;
      return out;
    }
    
    std::istream& load(std::istream& in){
      in >> start.x >> start.y >> end.x >> end.y;
      in >> color;
      in >> numSpikes >> height >> delay;
      timeaccu = delay;
      verts = NULL;
      timeaccu2 = (int)(delay*1.5f);
      verts2 = NULL;
      return in;
    }

  };
  static int compare(const void* a, const void* b){
    float fa = *(float*)a;
    float fb = *(float*)b;
    if (fa == fb)
      return 0;
    return fa < fb ? -1 : 1;
  }
  void drawFlash(int numVerts, Lightning* ltn, bool secondFlash){
    CGE::VertexBuffer* vb = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
    vb->create(VB_POSITION | VB_COLOR, numVerts);
    Vec2f dir = ltn->end-ltn->start;
    float length = dir.length();
    float* knots = new float[ltn->numSpikes];
    for (int i = 0; i < ltn->numSpikes; ++i){
      knots[i] = rand()/(float)RAND_MAX*length;
    }
    qsort(knots, ltn->numSpikes, sizeof(float), compare);
    dir.normalize();
    Vec2f ortho(dir.y, -dir.x);
    vb->lockVertexPointer();
    for (int i = 0; i < ltn->numSpikes; ++i){
      Vec2f vert = ltn->start+dir*knots[i];
      float tmp = rand()/(float)RAND_MAX;
      float height = ltn->height;
      float spikeheight = tmp*height*2-height;
      vb->setPosition(i + 1, CGE::Vec3f(vert.x + ortho.x*spikeheight, vert.y + ortho.y*spikeheight, 0));
      float opacity = 1.0f;
      if (secondFlash)
        opacity = 0.75f;
      vb->setColor(i + 1, CGE::Color(ltn->color.r / 255.0f, ltn->color.g / 255.0f, ltn->color.b / 255.0f, ltn->color.a / 255.0f * opacity));
    }
    delete [] knots;
    vb->setPosition(0, CGE::Vec3f(ltn->start.x, ltn->start.y, 0));
    vb->setPosition(numVerts-1, CGE::Vec3f(ltn->end.x, ltn->end.y, 0));
    float opacity = ltn->color.a / 255.0f;
    if (secondFlash)
      opacity = 0.25f;
    vb->setColor(0, CGE::Color(ltn->color.r / 255.0f, ltn->color.g / 255.0f, ltn->color.b / 255.0f, opacity));
    vb->setColor(numVerts-1, CGE::Color(ltn->color.r / 255.0f, ltn->color.g / 255.0f, ltn->color.b / 255.0f, opacity));
    vb->unlockVertexPointer();
    if (secondFlash){
      delete ltn->verts2;
      ltn->verts2 = vb;
    }
    else{
      delete ltn->verts;
      ltn->verts = vb;
    }
  }
  RenderableBlitObject* mFBO;
  CGE::Shader& mDrawShader;
  std::map<int,Lightning*> mLigthnings;
};

static const char fogfs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform float opacity;\n"
"uniform vec4 blendcol;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  color = texture2D(texture, tex_coord.st);\n"
"  color = mix(color, blendcol, opacity);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex : register(t0);\n"
"SamplerState sampl : register(s0);\n"
"\n"
"cbuffer perDraw{\n"
"  float opacity;\n"
"  float4 blendcol;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = float4(1.0, 1.0, 1.0, 1.0);\n"
"  color = tex.Sample(sampl, inp.tex_coord);\n"
"  color = lerp(color, blendcol, opacity);\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

class FogEffect : public PostProcessor::Effect{
public:
  FogEffect() : Effect(stdvs, fogfs){
    mName = "fog";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "opacity");
    int blendcol = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendcol");
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(blendcol, 0.5, 0.5, 0.5, 1.0);
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.deactivate();
  }
  virtual void deinit(){
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args,double);
    va_end(args);
    if (fade){
      mInterpolator.set(0, strength, 1000);
    }
    else{
      mInterpolator.set(strength, strength, 1000);
    }
    mFadeout = false;
    Effect::activate(fade);
  }
  virtual void deactivate(){
    if (mFade){
      mInterpolator.set(mInterpolator.current(), 0, mInterpolator.currTime());
      mFadeout = true;
    }
    else
      Effect::deactivate();
  }
  virtual bool update(unsigned time) {
    if (!mInterpolator.update(time)){
      if (mFadeout){
        Effect::deactivate();
        return false;
      }
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " ";
    mInterpolator.save(out);
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mFadeout;
    mInterpolator.load(in);
    return in;
  }
private:
  int mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
};

class ZoomEffect : public PostProcessor::Effect{
public:
  ZoomEffect() : Effect(druggedvs, stdfs), mVB(NULL){
    mName = "zoom";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mVB = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
    mVB->create(VB_POSITION | VB_TEXCOORD | VB_TEXCOORD2, 4);
    mShader.activate();
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.deactivate();
  }
  virtual void deinit(){
    delete mVB;
    mVB = NULL;
    Effect::deinit();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    mPos.x = va_arg(args, int);
    mPos.y = va_arg(args, int);
    mScale = (float)va_arg(args, double);
    mFadetime = va_arg(args, int);
    va_end(args);
    Effect::activate(fade);
    mFadeout = false;
    setupZoom();
  }
  virtual void deactivate(){
    mFadeout = true;
    for (int i = 0; i < 8; ++i){
      mVerts[i].set(mVerts[i].current(), 0, (float)mFadetime);
    }
  }
  virtual bool update(unsigned time) {
    int finishCount = 0;
    for (int i = 0; i < 8; ++i){
      if (!mVerts[i].update(time)){
        if (mFadeout)
          ++finishCount;
      }
    }
    if (finishCount >= 8){
      Effect::deactivate();
      return false;
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    float tmp[8];
    for (int i = 0; i < 8; ++i){
      tmp[i] = mVerts[i].current();
      if (i == 0 || i == 2 || i == 3 || i == 7)
        tmp[i] *= -1;
    }
    mVB->lockVertexPointer();
    Engine::instance()->fillWithStdQuad(mVB);
    mVB->setTexCoord2(0, CGE::Vec2f(tmp[0], tmp[1]));
    mVB->setTexCoord2(1, CGE::Vec2f(tmp[2], tmp[3]));
    mVB->setTexCoord2(2, CGE::Vec2f(tmp[4], tmp[5]));
    mVB->setTexCoord2(3, CGE::Vec2f(tmp[6], tmp[7]));
    mVB->unlockVertexPointer();
    mVB->activate();
    mVB->draw(CGE::VB_Tristrip, NULL);
    mShader.deactivate();
    Engine::instance()->restoreRenderDefaults();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " " << mPos.x << " " << mPos.y << " " << mScale << " " << mFadetime;
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mFadeout >> mPos.x >> mPos.y >> mScale >> mFadetime;
    int old = mFadetime;
    mFadetime = 0;
    setupZoom();
    mFadetime = old;
    return in;
  }
private:
  void setupZoom(){
    float xres = (float)Engine::instance()->getResolution().x;
    float sx = ((mPos.x-xres/2)/xres)*2;
    float yres = (float)Engine::instance()->getResolution().y;
    float y = (float)mPos.y;
    if (CGE::Engine::instance()->getRenderer()->getRenderType() == CGE::DirectX)
      y = yres - mPos.y;
    float sy = ((y-yres/2)/yres)*2;
    float scale = mScale;
    float transx = sx*(1-scale);
    float transy = sy*(1-scale);
    for (int i = 0; i < 8; ++i){
      float time = (float)mFadetime;
      float tmp = scale-1;
      if (i == 0 || i == 2)
        tmp -= transx;
      if (i == 1 || i == 5)
        tmp += transy;
      if (i == 3 || i == 7)
        tmp -= transy;
      if (i == 4 || i == 6)
        tmp += transx;
      mVerts[i].set(0, tmp, time);
    }
  }
  bool mFadeout;
  CGE::VertexBuffer* mVB;
  Interpolator mVerts[8];
  Vec2i mPos;
  float mScale;
  int mFadetime;
};

class FlashEffect : public PostProcessor::Effect{
public:
  FlashEffect() : Effect(stdvs, fogfs){
    mName = "whiteflash";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "opacity");
    int blendcol = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendcol");
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(blendcol, 1.0, 1.0, 1.0, 1.0);
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.deactivate();
  }
  virtual void deinit(){
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    int fadein = va_arg(args, int);
    int fadeout = va_arg(args, int);
    va_end(args);
    mInterpolator.set(0, 1.0f, (float)fadein);
    mFadeout = false;
    mFadeoutTime = fadeout;
    Effect::activate(fade);
  }
  virtual void deactivate(){
    mInterpolator.set(mInterpolator.current(), 0, (float)mFadeoutTime);
    mFadeout = true;
  }
  virtual bool update(unsigned time) {
    if (!mInterpolator.update(time)){
      if (mFadeout){
        Effect::deactivate();
        return false;
      }
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " " << mFadeoutTime << " ";
    mInterpolator.save(out);
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mFadeout >> mFadeoutTime;
    mInterpolator.load(in);
    return in;
  }
private:
  int mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
  int mFadeoutTime;
};

static const char pixelatefs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform float strength;\n"
"uniform vec2 cam_offset;\n"
"uniform vec2 pixel_offset;\n"
"\n"
"void main(){\n"
"  vec2 delta = strength*pixel_offset;\n"
"  vec2 basecoord = delta*floor(tex_coord/delta);\n"
"  vec4 color = vec4(0,0,0,0);\n"
"  for (int i = -cam_offset.x; i < strength-cam_offset.x; ++i){\n"
"    for (int j = -cam_offset.y; j < strength-cam_offset.y; ++j){\n"
"      vec2 coord = basecoord + vec2(i*pixel_offset.x, j*pixel_offset.y);\n"
"      color += texture2D(texture, clamp(coord, pixel_offset, vec2(1)));\n"
"    }\n"
"  }\n"
"  color /= strength*strength;\n"
"  gl_FragColor = vec4(color.rgb, 1.0);\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float strength;\n"
"  float2 cam_offset;\n"
"}\n"
"cbuffer perInstance{\n"
"  float2 pixel_offset;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float2 delta = strength*pixel_offset;\n"
"  float2 basecoord = delta*floor(inp.tex_coord/delta);\n"
"  float4 color = float4(0,0,0,0);\n"
"  for (int i = -cam_offset.x; i < strength-cam_offset.x; ++i){\n"
"    for (int j = -cam_offset.y; j < strength-cam_offset.y; ++j){\n"
"      float2 coord = basecoord + float2(i*pixel_offset.x, j*pixel_offset.y);\n"
"      color += tex.Sample(sampl, clamp(coord, pixel_offset, float2(1,1)));\n"
"    }\n"
"  }\n"
"  color /= strength*strength;\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";

class PixelateEffect : public PostProcessor::Effect{
public:
  PixelateEffect() : Effect(stdvs, pixelatefs){
    mName = "pixelate";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "tex");
    mShader.uniform(tex, 0);
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    mShader.uniform(scale, size.x / powx, size.y / powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "strength");
    mCamOffsetLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "cam_offset");
    int pixeloffset = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "pixel_offset");
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER, 1);
    mShader.uniform(pixeloffset, 1.0f / powx, 1.0f / powy);
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER, 1);
    mShader.deactivate();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    mPixSize = (float)va_arg(args, double);
    Effect::activate(fade);
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    Vec2i camOffset = -Engine::instance()->getRoom("")->getScrollOffset();
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(mIntensityLoc, mPixSize);
    mShader.uniform(mCamOffsetLoc, fmodf((float)camOffset.x, mPixSize), fmodf((float)camOffset.y, mPixSize));
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mPixSize << " ";
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mPixSize;
    return in;
  }
private:
  int mIntensityLoc;
  int mCamOffsetLoc;
  float mPixSize;
};

static const char scanlinefs[] =
"@GLSL"
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform float strength;\n"
"uniform vec2 pixel_size;\n"
"\n"
"void main(){\n"
"  vec4 color = texture2D(texture, tex_coord);\n"
"  float rem = fmod(tex_coord.y, 3*pixel_size.y);\n"
"  if (rem < pixel_size.y)\n"
"    color *= strength;\n"
"  gl_FragColor = vec4(color.rgb, 1.0);\n"
"}\n"
""
"@HLSL"
""
"Texture2D tex;\n"
"SamplerState sampl;\n"
"\n"
"cbuffer perDraw{\n"
"  float strength;\n"
"}\n"
"cbuffer perInstance{\n"
"  float2 pixel_size;\n"
"}\n"
"\n"
"struct PSInput{\n"
"  float4 vPos : SV_POSITION;\n"
"  float2 tex_coord : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main(PSInput inp) : SV_TARGET {\n"
"  float4 color = tex.Sample(sampl, inp.tex_coord);\n"
"  float rem = fmod(inp.tex_coord.y, 3*pixel_size.y);\n"
"  if (rem < pixel_size.y)\n"
"    color *= strength;\n"
"  return float4(color.rgb, 1.0);\n"
"}\n"
"\n";


class ScanlineEffect : public PostProcessor::Effect{
public:
  ScanlineEffect() : Effect(stdvs, scanlinefs){
    mName = "scanlines";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    int tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "tex");
    mShader.uniform(tex, 0);
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.lockUniforms(CGE::Shader::VERTEX_SHADER);
    int scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    mShader.uniform(scale, size.x / powx, size.y / powy);
    mShader.unlockUniforms(CGE::Shader::VERTEX_SHADER);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "strength");
    int pixeloffset = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "pixel_size");
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER, 1);
    mShader.uniform(pixeloffset, 1.0f / powx, 1.0f / powy);
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER, 1);
    mShader.deactivate();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args, double);
    va_end(args);
    mInterpolator.set(fade ? 1.0f : 1.0f-strength, 1.0f-strength, 1000);
    mFadeout = false;
    Effect::activate(fade);
  }
  virtual void deactivate(){
    if (mFade){
      mInterpolator.set(mInterpolator.current(), 1.0f, 1000);
      mFadeout = true;
    }
    else
      Effect::deactivate();
  }
  virtual bool update(unsigned time) {
    if (!mInterpolator.update(time)){
      if (mFadeout){
        Effect::deactivate();
        return false;
      }
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    input->getTexture()->activate();
    CGE::Engine::instance()->getRenderer()->clear(COLORBUFFER | ZBUFFER);
    mShader.activate();
    mShader.lockUniforms(CGE::Shader::FRAGMENT_SHADER);
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    mShader.unlockUniforms(CGE::Shader::FRAGMENT_SHADER);
    Engine::instance()->drawQuad();
    mShader.deactivate();
  }
  virtual std::ostream& save(std::ostream& out){
    Effect::save(out);
    out << mFadeout << " ";
    mInterpolator.save(out);
    return out;
  }
  virtual std::istream& load(std::istream& in){
    Effect::load(in);
    in >> mFadeout;
    mInterpolator.load(in);
    return in;
  }
private:
  int mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
};


/* Postprocessor */

#define REGISTER_EFFECT(effect, class, ...) class* effect = new class(__VA_ARGS__); mEffects[effect->getName()] = effect;

PostProcessor::PostProcessor(int width, int height, int depth) : mResult1(width, height, depth), mResult2(width, height, depth){
  mResult1.realize();
  mResult2.realize();
  REGISTER_EFFECT(darkbloom, BloomEffect, "darkbloom", darkbloomfs);
  REGISTER_EFFECT(noise, NoiseEffect, "noise", noisefs, 1);
  REGISTER_EFFECT(hell, BloomEffect, "hell", hellfs);
  REGISTER_EFFECT(motionblur, MotionBlurEffect, "motionblur", motionblurfs);
  REGISTER_EFFECT(heat, HeatEffect, "heat", HeatEffect::RANDOM);
  REGISTER_EFFECT(whoosh, BloomEffect, "whoosh", whooshfs);
  REGISTER_EFFECT(bloom, BloomEffect, "bloom", bloomfs)
  REGISTER_EFFECT(drugged, DruggedEffect);
  REGISTER_EFFECT(lightning, LightningEffect);
  REGISTER_EFFECT(fog, FogEffect);
  REGISTER_EFFECT(zoom, ZoomEffect);
  REGISTER_EFFECT(flash, FlashEffect);
  REGISTER_EFFECT(underwater, HeatEffect, "underwater", HeatEffect::SINE);
  REGISTER_EFFECT(colornoise, NoiseEffect, "colornoise", clrnoisefs, 4);
  REGISTER_EFFECT(pixelate, PixelateEffect);
  REGISTER_EFFECT(scanlines, ScanlineEffect);
  REGISTER_EFFECT(warpspeed, MotionBlurEffect, "warpspeed", motionblurfs);
}

PostProcessor::~PostProcessor(){
  for (std::map<std::string,Effect*>::iterator iter = mEffects.begin(); iter != mEffects.end(); ++iter){
    iter->second->deinit();
    delete iter->second;
  }
}

BlitObject* PostProcessor::process(BlitObject* input, unsigned time){
  if (mActiveEffects.empty())
    return input;
  RenderableBlitObject* target = &mResult1;
  BlitObject* source = input;
  for (std::list<Effect*>::iterator iter = mActiveEffects.begin(); iter != mActiveEffects.end(); ){
    std::list<Effect*>::iterator iter2 = iter;
    ++iter2;
    if ((*iter)->update(time)){
      target->bind();
      (*iter)->apply(source);
      target->unbind();
      //swap source and target buffers for next pass
      if (iter2 != mActiveEffects.end()){
        RenderableBlitObject* tmp = target;
        if (source == input)
          target = &mResult2;
        else
          target = (RenderableBlitObject*)source;
        source = tmp;
      }
    }
    iter = iter2;
  }
  return target;
}

PostProcessor::Effect* PostProcessor::getEffect(const std::string& effect){
  for (std::list<Effect*>::iterator iter = mActiveEffects.begin(); iter != mActiveEffects.end(); ++iter){
    if ((*iter)->getName() == effect)
      return *iter;
  }
  std::map<std::string,Effect*>::iterator iter = mEffects.find(effect);
  if (iter != mEffects.end()){
    return iter->second;
  }
  return NULL;
}

void PostProcessor::stopEffects(){
  for (std::list<Effect*>::iterator iter = mActiveEffects.begin(); iter != mActiveEffects.end(); ){
    std::list<Effect*>::iterator iter2 = iter;
    ++iter2;
    (*iter)->deactivate();
    iter = iter2;
  }
}

std::ostream& PostProcessor::save(std::ostream& out){
  out << mActiveEffects.size() << "\n";
  for (std::list<Effect*>::iterator iter = mActiveEffects.begin(); iter != mActiveEffects.end(); ++iter){
    (*iter)->save(out);
    out << std::endl;
  }
  return out;
}

std::istream& PostProcessor::load(std::istream& in){
  stopEffects();
  int size;
  in >> size;
  std::string name;
  for (int i = 0; i < size; ++i){
    in >> name;
    PostProcessor::Effect* ef = getEffect(name);
    ef->load(in);
    ef->activate();
  }
  return in;
}
