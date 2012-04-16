#include "PostProcessing.h"

#include "Renderer.h"
#include "Engine.h"

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
private:
  float mSource;
  float mTarget;
  float mCurrent;
  float mTime;
  int mTimeAccu;
};

PostProcessor::Effect::Effect(const char* vertexsource, const char* fragmentsource) : mFade(false){
  mShader.addShader(GL_VERTEX_SHADER, vertexsource);
  mShader.addShader(GL_FRAGMENT_SHADER, fragmentsource);
  mShader.linkShaders();
}

PostProcessor::Effect::~Effect(){
  mShader.deleteShaders();
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

/*dark bloom*/

static const char bloomvs[] =
"precision mediump float;\n"
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
"  tex_coord2 = vec2(position.x*2.0-1.0, (0.0+position.y)*2.0-1.0)*pixel_offset*2;\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
"";

static const char darkbloomfs[] =
"precision mediump float;\n"
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
"precision mediump float;\n"
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

class DarkBloomEffect : public PostProcessor::Effect{
public:
  DarkBloomEffect(const char* name, const char* fragmentshader) : Effect(bloomvs, fragmentshader){
    mName = name;
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mShader.activate();
    GLint tex = mShader.getUniformLocation("texture");
    mShader.uniform(tex, 0);
    GLint scale = mShader.getUniformLocation("tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    GLint pixeloffset = mShader.getUniformLocation("pixel_offset");
    mShader.uniform(pixeloffset, 1.0f/size.x, 1.0f/size.y);
    mIntensityLoc = mShader.getUniformLocation("strength");
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
"precision mediump float;\n"
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
"precision mediump float;\n"
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
"precision mediump float;\n"
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
    GLint tex = mShader.getUniformLocation("texture");
    mShader.uniform(tex, 0);
    GLint blendtex = mShader.getUniformLocation("blendtex");
    mShader.uniform(blendtex, 1);
    GLint scale = mShader.getUniformLocation("tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    mIntensityLoc = mShader.getUniformLocation("opacity");
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
private:
  GLint mIntensityLoc;
  Interpolator mInterpolator;
  bool mFadeout;
  GLuint mBlendTex;
  CGE::Image mImage;
};

static const char motionblurfs[] =
"precision mediump float;\n"
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
    mStdShader.addShader(GL_VERTEX_SHADER, stdvs);
    mStdShader.addShader(GL_FRAGMENT_SHADER, stdfs);
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
    GLint tex = mShader.getUniformLocation("texture");
    mShader.uniform(tex, 0);
    GLint scale = mShader.getUniformLocation("tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    GLint motion = mShader.getUniformLocation("motion1");
    mShader.uniform(motion, 1);
    motion = mShader.getUniformLocation("motion2");
    mShader.uniform(motion, 2);
    motion = mShader.getUniformLocation("motion3");
    mShader.uniform(motion, 3);
    mShader.deactivate();
    mStdShader.activate();
    tex = mStdShader.getUniformLocation("texture");
    mStdShader.uniform(tex, 0);
    scale = mStdShader.getUniformLocation("tex_scale");
    mStdShader.uniform(scale, size.x/powx, size.y/powy);
    mStdShader.deactivate();
    for (int i = 0; i < 3; ++i){
      RenderableBlitObject* rbo = new RenderableBlitObject(Engine::instance()->getResolution().x, Engine::instance()->getResolution().y, 0);
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
private:
  GL2Shader mStdShader;
  std::list<RenderableBlitObject*> mPrevFrames;
  std::list<RenderableBlitObject*> mGenFrames;
  int mTakeFrame;
  int mTakeCount;
};

static const char heatfs[] =
"precision mediump float;\n"
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
"  float xcoord = clamp(tex_coord.s+xoffset*pixel_offset.x, 0.0, tex_scale.x-pixel_offset.x/2);"
"  color = texture2D(texture, vec2(xcoord,tex_coord.t));\n"
"  gl_FragColor = color;\n"
"  gl_FragColor.a = 1.0;\n"
"}\n"
"";

class HeatEffect : public PostProcessor::Effect{
public:
  HeatEffect() : Effect(stdvs, heatfs), mBlendTex(0), mTimeAccu(0){
    mName = "heat";
  }
  virtual void init(const Vec2f& size){
    mImage.setFormat(1, 1, (int)size.y/2);
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
    GLint tex = mShader.getUniformLocation("texture");
    mShader.uniform(tex, 0);
    GLint blendtex = mShader.getUniformLocation("blendtex");
    mShader.uniform(blendtex, 1);
    GLint scale = mShader.getUniformLocation("tex_scale");
    float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
    float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
    mShader.uniform(scale, size.x/powx, size.y/powy);
    GLint pixeloffset = mShader.getUniformLocation("pixel_offset");
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
        mImage.getData()[i] = mFadeout ? 127 : (unsigned char)((rand()/(float)RAND_MAX)*255);
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
private:
  bool mFadeout;
  GLuint mBlendTex;
  CGE::Image mImage;
  int mTimeAccu;
  int mFadeoutPixels;
};


/* Postprocessor */

#define REGISTER_EFFECT(effect, class, ...) class* effect = new class(__VA_ARGS__); mEffects[effect->getName()] = effect;

PostProcessor::PostProcessor(int width, int height, int depth) : mResult1(width, height, depth), mResult2(width, height, depth){
  REGISTER_EFFECT(darkbloom, DarkBloomEffect, "darkbloom", darkbloomfs);
  REGISTER_EFFECT(noise, NoiseEffect);
  REGISTER_EFFECT(hell, DarkBloomEffect, "hell", hellfs);
  REGISTER_EFFECT(motionblur, MotionBlurEffect);
  REGISTER_EFFECT(heat, HeatEffect);
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
