#include "PostProcessing.h"

#include "Renderer.h"
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

PostProcessor::Effect::Effect(const char* vertexsource, const char* fragmentsource) : mFade(false){
  mShader.bindAttribLocation(0, "position");
  mShader.bindAttribLocation(1, "texCoord");
  mShader.addShader(CGE::Shader::VERTEX_SHADER, vertexsource);
  mShader.addShader(CGE::Shader::FRAGMENT_SHADER, fragmentsource);
  mShader.linkShaders();
}

PostProcessor::Effect::~Effect(){
  
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
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"\n"
"uniform vec2 tex_scale;\n"
"uniform vec2 pixel_offset;\n"
"\n"
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*tex_scale.x, (0.0+position.y)*tex_scale.y);\n"
"  tex_coord2 = vec2(position.x*2.0-1.0, (0.0+position.y)*2.0-1.0)*pixel_offset*2.0;\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
"";

static const char darkbloomfs[] =
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
"";

static const char hellfs[] =
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
"";

static const char whooshfs[] =
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
"";

static const char bloomfs[] =
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
"  color = color+texture2D(texture, tex_coord.st-tex_coord2.st)*intensity;\n"
"  color = color+texture2D(texture, tex_coord.st-tex_coord2.st*vec2(2.0))*intensity;\n"
"  color = color+texture2D(texture, tex_coord.st-tex_coord2.st*vec2(3.0))*intensity;\n"
"  color = color+texture2D(texture, tex_coord.st-tex_coord2.st*vec2(4.0))*intensity;\n"
"  color = color+texture2D(texture, tex_coord.st-tex_coord2.st*vec2(5.0))*intensity;\n"
"  color = color+texture2D(texture, tex_coord.st-tex_coord2.st*vec2(6.0))*intensity;\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

class BloomEffect : public PostProcessor::Effect{
public:
  BloomEffect(const char* name, const char* fragmentshader) : Effect(bloomvs, fragmentshader){
    mName = name;
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    GLint pixeloffset = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "pixel_offset");
    mShader.uniform(pixeloffset, 1.0f/size.x, 1.0f/size.y);
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
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  GLint mIntensityLoc;
  Interpolator mInterpolator;
  State mState;
};

/*noise*/

static const char stdvs[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"\n"
"uniform vec2 tex_scale;\n"
"\n"
"varying vec2 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*tex_scale.x, (0.0+position.y)*tex_scale.y);\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
"";

static const char stdfs[] =
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
"";

static const char noisefs[] =
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
"";

class NoiseEffect : public PostProcessor::Effect{
public:
  NoiseEffect() : Effect(stdvs, noisefs), mBlendTex(0){
    mName = "noise";
  }
  virtual void init(const Vec2f& size){
    mImage.setFormat(1, (int)size.x/2, (int)size.y/2);
    mImage.allocateData();
    for (unsigned i = 0; i < mImage.getImageSize(); ++i){
      mImage.getData()[i] = (unsigned char)((rand()/(float)RAND_MAX)*255);
    }
    Vec2i imgsize;
    Vec2f imgscale;
    glActiveTexture(GL_TEXTURE1);
    mBlendTex = Engine::instance()->genTexture(&mImage, imgsize, imgscale);
    glActiveTexture(GL_TEXTURE0);
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint blendtex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendtex");
    mShader.uniform(blendtex, 1);
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "opacity");
    mShader.deactivate();
  }
  virtual void deinit(){
    if (mBlendTex != 0)
      glDeleteTextures(1, &mBlendTex);
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
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mBlendTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mImage.getWidth(), mImage.getHeight(), GL_LUMINANCE, GL_UNSIGNED_BYTE, mImage.getData());
    glActiveTexture(GL_TEXTURE0);
    return true;
  }
  virtual void apply(BlitObject* input){
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  GLint mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
  GLuint mBlendTex;
  CGE::Image mImage;
};

static const char motionblurfs[] =
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
"uniform float opacity;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  vec4 blendcol = vec4(1.0);\n"
"  color.rgb = texture2D(motion3, tex_coord.st).rgb;\n"
//"  color = mix(color, blendcol, 0.5);\n"
"  blendcol.rgb = texture2D(motion2, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, 0.5);\n"
"  blendcol.rgb = texture2D(motion1, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, 0.5);\n"
"  blendcol.rgb = texture2D(texture, tex_coord.st).rgb;\n"
"  color = mix(color, blendcol, 0.5);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

class MotionBlurEffect : public PostProcessor::Effect{
public:
  MotionBlurEffect() : Effect(stdvs, motionblurfs){
    mName = "motionblur";
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
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    GLint motion = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "motion1");
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
    mStdShader.uniform(scale, size.x/powx, size.y/powy);
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
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
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
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
      glActiveTexture(GL_TEXTURE1+count);
      glBindTexture(GL_TEXTURE_2D, (*iter)->getTexture());
      ++count;
    }
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glActiveTexture(GL_TEXTURE0);
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
  GL2Shader mStdShader;
  std::list<RenderableBlitObject*> mPrevFrames;
  std::list<RenderableBlitObject*> mGenFrames;
  int mTakeFrame;
  int mTakeCount;
};

static const char heatfs[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"varying vec2 tex_coord;\n"
"\n"
"uniform sampler2D texture;\n"
"uniform sampler2D blendtex;\n"
"uniform float opacity;\n"
"uniform vec2 tex_scale;\n"
"uniform vec2 pixel_offset;\n"
"\n"
"void main(){\n"
"  float xoffset = texture2D(blendtex, vec2(0.5, tex_coord.t)).r*2.0-1.0;\n"
"  vec4 color = vec4(1.0);\n"
"  float xcoord = clamp(tex_coord.s+xoffset*pixel_offset.x, 0.0, tex_scale.x-pixel_offset.x/2.0);"
"  color = texture2D(texture, vec2(xcoord,tex_coord.t));\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

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
    glActiveTexture(GL_TEXTURE1);
    mBlendTex = Engine::instance()->genTexture(&mImage, imgsize, imgscale);
    glActiveTexture(GL_TEXTURE0);
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint blendtex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendtex");
    mShader.uniform(blendtex, 1);
    GLint scale = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    GLint pixeloffset = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "pixel_offset");
    mShader.uniform(pixeloffset, 1.0f/size.x, 1.0f/size.y);
    mShader.deactivate();
  }
  virtual void deinit(){
    if (mBlendTex != 0)
      glDeleteTextures(1, &mBlendTex);
  }
  virtual void activate(bool fade, ...){
    Effect::activate(fade);
    if (fade){
      memset(mImage.getData(), 127, mImage.getImageSize());
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, mBlendTex);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mImage.getWidth(), mImage.getHeight(), GL_LUMINANCE, GL_UNSIGNED_BYTE, mImage.getData());
      glActiveTexture(GL_TEXTURE0);
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
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, mBlendTex);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mImage.getWidth(), mImage.getHeight(), GL_LUMINANCE, GL_UNSIGNED_BYTE, mImage.getData());
      glActiveTexture(GL_TEXTURE0);
      mTimeAccu -= 50;
      if (mFadeout)
        mFadeoutPixels -= time;
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  GLuint mBlendTex;
  CGE::Image mImage;
  int mTimeAccu;
  int mFadeoutPixels;
  int mLineCount;
  Type mType;
};

static const char druggedvs[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"attribute vec2 offset;\n"
"\n"
"uniform vec2 tex_scale;\n"
"\n"
"varying vec2 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*tex_scale.x, (0.0+position.y)*tex_scale.y);\n"
"  gl_Position = vec4(position.x*2.0-1.0+offset.x, position.y*2.0-1.0+offset.y, 0.0, 1.0);\n"
"}\n"
"";

class DruggedEffect : public PostProcessor::Effect{
public:
  DruggedEffect() : Effect(druggedvs, stdfs){
    mName = "drugged";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mOffsetIndex = mShader.getAttribLocation("offset");
    mShader.deactivate();
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
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    float tmp[8];
    for (int i = 0; i < 8; ++i){
      tmp[i] = mVerts[i].current();
      if (i == 0 || i == 2 || i == 3 || i == 7)
        tmp[i] *= -1;
    }
    glVertexAttribPointer(mOffsetIndex, 2, GL_FLOAT, GL_FALSE, 0, tmp);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  GLuint mOffsetIndex;
  bool mFadeout;
  Interpolator mVerts[8];
  float mStrength;
};

static const char lightningfs[] =
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
"  blendcol = texture2D(blendtex, tex_coord.st);\n"
"  color = mix(color, blendcol, blendcol.a);\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

static const char drawvs[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 position;\n"
"attribute vec4 color;\n"
"\n"
"varying vec4 vert_color;\n"
"\n"
"void main(){\n"
"  vert_color = color;\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
"";

static const char drawfs[] =
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
"";

class LightningEffect : public PostProcessor::Effect{
public:
  LightningEffect() : Effect(stdvs, lightningfs), mFBO(NULL){
    mName = "lightning";
    mDrawShader.addShader(CGE::Shader::VERTEX_SHADER, drawvs);
    mDrawShader.addShader(CGE::Shader::FRAGMENT_SHADER, drawfs);
    mDrawShader.bindAttribLocation(0, "position");
    mDrawShader.bindAttribLocation(1, "color");
    mDrawShader.linkShaders();
  }
  ~LightningEffect(){
    deactivate();
  }
  virtual void init(const Vec2f& size){
    glActiveTexture(GL_TEXTURE1);
    mFBO = new RenderableBlitObject((int)size.x, (int)size.y, 0);
    mFBO->realize();
    Vec2i imgsize;
    Vec2f imgscale;
    glActiveTexture(GL_TEXTURE0);
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint blendtex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendtex");
    mShader.uniform(blendtex, 1);
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
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
    ltn.colorAttrib = NULL;
    ltn.timeaccu2 = (int)(ltn.delay*1.5f);
    ltn.verts2 = NULL;
    ltn.colorAttrib2 = NULL;
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
    glActiveTexture(GL_TEXTURE1);
    
    mFBO->bind();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, iter->second->verts);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, iter->second->colorAttrib);
      glDrawArrays(GL_LINE_STRIP, 0, numVerts);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, iter->second->verts2);
      glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, iter->second->colorAttrib2);
      glDrawArrays(GL_LINE_STRIP, 0, numVerts);
    }
    mDrawShader.deactivate();
    Engine::instance()->restoreRenderDefaults();
    mFBO->unbind();
    
    glBindTexture(GL_TEXTURE_2D, mFBO->getTexture());
    glActiveTexture(GL_TEXTURE0);
    return true;
  }
  virtual void apply(BlitObject* input){
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    //mShader.uniform(mIntensityLoc, mInterpolator.current());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    float* verts;
    float* colorAttrib;
    int timeaccu2;
    float* verts2;
    float* colorAttrib2;

    ~Lightning(){
      delete [] verts;
      delete [] colorAttrib;
      delete [] verts2;
      delete [] colorAttrib2;
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
      colorAttrib = NULL;
      timeaccu2 = (int)(delay*1.5f);
      verts2 = NULL;
      colorAttrib2 = NULL;
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
    float* verts = new float[numVerts*2];
    float* color = new float[numVerts*4];
    Vec2f dir = ltn->end-ltn->start;
    float length = dir.length();
    float* knots = new float[ltn->numSpikes];
    for (int i = 0; i < ltn->numSpikes; ++i){
      knots[i] = rand()/(float)RAND_MAX*length;
    }
    qsort(knots, ltn->numSpikes, sizeof(float), compare);
    dir.normalize();
    Vec2f ortho(dir.y, -dir.x);
    for (int i = 0; i < ltn->numSpikes; ++i){
      Vec2f vert = ltn->start+dir*knots[i];
      float tmp = rand()/(float)RAND_MAX;
      float height = ltn->height;
      float spikeheight = tmp*height*2-height;
      verts[2*(i+1)] = vert.x+ortho.x*spikeheight;
      verts[2*(i+1)+1] = vert.y+ortho.y*spikeheight;
      color[4*(i+1)] = ltn->color.r/255.0f; color[4*(i+1)+1] = ltn->color.g/255.0f; color[4*(i+1)+2] = ltn->color.b/255.0f; color[4*(i+1)+3] = ltn->color.a/255.0f;
      if (secondFlash)
        color[4*(i+1)+3] *= 0.75f;
    }
    delete [] knots;
    verts[0] = ltn->start.x; verts[1] = ltn->start.y;
    verts[2*numVerts-2] = ltn->end.x; verts[2*numVerts-1] = ltn->end.y;
    color[0] = ltn->color.r/255.0f; color[1] = ltn->color.g/255.0f; color[2] = ltn->color.b/255.0f; color[3] = ltn->color.a/255.0f;
    if (secondFlash)
      color[3] = 0.25f;
    color[4*numVerts-4] = ltn->color.r/255.0f; color[4*numVerts-3] = ltn->color.g/255.0f; color[4*numVerts-2] = ltn->color.b/255.0f; color[4*numVerts-1] = ltn->color.a/255.0f;
    if (secondFlash)
      color[4*numVerts-1] = 0.25f;
    if (secondFlash){
      delete [] ltn->verts2;
      delete [] ltn->colorAttrib2;
      ltn->verts2 = verts;
      ltn->colorAttrib2 = color;
    }
    else{
      delete [] ltn->verts;
      delete [] ltn->colorAttrib;
      ltn->verts = verts;
      ltn->colorAttrib = color;
    }
  }
  RenderableBlitObject* mFBO;
  GL2Shader mDrawShader;
  std::map<int,Lightning*> mLigthnings;
};

static const char fogfs[] =
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
"";

class FogEffect : public PostProcessor::Effect{
public:
  FogEffect() : Effect(stdvs, fogfs){
    mName = "fog";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "opacity");
    GLint blendcol = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendcol");
    mShader.uniform(blendcol, 0.5, 0.5, 0.5, 1.0);
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
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  GLint mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
};

static const char zoomvs[] =
#ifdef RENDER_TEGRA
"precision mediump float;\n"
#endif
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"attribute vec2 offset;\n"
"\n"
"uniform vec2 tex_scale;\n"
"\n"
"varying vec2 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*tex_scale.x, (0.0+position.y)*tex_scale.y);\n"
"  gl_Position = vec4(position.x*2.0-1.0+offset.x, position.y*2.0-1.0+offset.y, 0.0, 1.0);\n"
"}\n"
"";

class ZoomEffect : public PostProcessor::Effect{
public:
  ZoomEffect() : Effect(zoomvs, stdfs){
    mName = "zoom";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mOffsetIndex = mShader.getAttribLocation("offset");
    mShader.deactivate();
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
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    float tmp[8];
    for (int i = 0; i < 8; ++i){
      tmp[i] = mVerts[i].current();
      if (i == 0 || i == 2 || i == 3 || i == 7)
        tmp[i] *= -1;
    }
    glVertexAttribPointer(mOffsetIndex, 2, GL_FLOAT, GL_FALSE, 0, tmp);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    float sy = ((mPos.y-yres/2)/yres)*2;
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
  GLuint mOffsetIndex;
  bool mFadeout;
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
    GLint tex = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "texture");
    mShader.uniform(tex, 0);
    GLint scale = mShader.getUniformLocation(CGE::Shader::VERTEX_SHADER, "tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mIntensityLoc = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "opacity");
    GLint blendcol = mShader.getUniformLocation(CGE::Shader::FRAGMENT_SHADER, "blendcol");
    mShader.uniform(blendcol, 1.0, 1.0, 1.0, 1.0);
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
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  GLint mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
  int mFadeoutTime;
};


/* Postprocessor */

#define REGISTER_EFFECT(effect, class, ...) class* effect = new class(__VA_ARGS__); mEffects[effect->getName()] = effect;

PostProcessor::PostProcessor(int width, int height, int depth) : mResult1(width, height, depth), mResult2(width, height, depth){
  mResult1.realize();
  mResult2.realize();
  REGISTER_EFFECT(darkbloom, BloomEffect, "darkbloom", darkbloomfs);
  REGISTER_EFFECT(noise, NoiseEffect);
  REGISTER_EFFECT(hell, BloomEffect, "hell", hellfs);
  REGISTER_EFFECT(motionblur, MotionBlurEffect);
  REGISTER_EFFECT(heat, HeatEffect, "heat", HeatEffect::RANDOM);
  REGISTER_EFFECT(whoosh, BloomEffect, "whoosh", whooshfs);
  REGISTER_EFFECT(bloom, BloomEffect, "bloom", bloomfs)
  REGISTER_EFFECT(drugged, DruggedEffect);
  REGISTER_EFFECT(lightning, LightningEffect);
  REGISTER_EFFECT(fog, FogEffect);
  REGISTER_EFFECT(zoom, ZoomEffect);
  REGISTER_EFFECT(flash, FlashEffect);
  REGISTER_EFFECT(underwater, HeatEffect, "underwater", HeatEffect::SINE);
}

PostProcessor::~PostProcessor(){
  for (std::map<std::string,Effect*>::iterator iter = mEffects.begin(); iter != mEffects.end(); ++iter){
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