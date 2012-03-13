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
"  vec4 intensity = vec4(strength.xxx,0.0)/vec4(6.0);\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st)*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(2.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(3.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(4.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(5.0))*intensity;\n"
"  color = color-texture2D(texture, tex_coord.st-tex_coord2.st*vec2(6.0))*intensity;\n"
"  gl_FragColor = color;\n"
"  gl_FragDepth = gl_FragCoord.z;\n"
"}\n"
"";

class DarkBloomEffect : public PostProcessor::Effect{
public:
  DarkBloomEffect() : Effect(bloomvs, darkbloomfs){
    mName = "darkbloom";
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
    mAnimate = va_arg(args,bool);
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

static const char noisevs[] =
"precision mediump float;\n"
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"\n"
"uniform vec2 tex_scale;\n"
"uniform vec2 pixel_offset;\n"
"\n"
"varying vec2 tex_coord;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*tex_scale.x, (0.0+position.y)*tex_scale.y);\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
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
"  gl_FragDepth = gl_FragCoord.z;\n"
"}\n"
"";

class NoiseEffect : public PostProcessor::Effect{
public:
  NoiseEffect() : Effect(noisevs, noisefs){
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
    GLint pixeloffset = mShader.getUniformLocation("pixel_offset");
    mShader.uniform(pixeloffset, 1.0f/size.x, 1.0f/size.y);
    mIntensityLoc = mShader.getUniformLocation("opacity");
    mShader.deactivate();
  }
  virtual void deinit(){
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

/* Postprocessor */

#define REGISTER_EFFECT(effect, class) class* effect = new class(); mEffects[effect->getName()] = effect;

PostProcessor::PostProcessor(int width, int height, int depth) : mResult1(width, height, depth), mResult2(width, height, depth){
  REGISTER_EFFECT(darkbloom, DarkBloomEffect);
  REGISTER_EFFECT(noise, NoiseEffect);
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
