#include "PostProcessing.h"

#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstring>

#include <renderer/renderdevice.h>
#include <renderer/shaderlibrary.h>

#include "Engine.h"
#include "Graphics.h"
#include <system/allocation.h>

using namespace adv;

// The shaders of the effects are in shaders/ (adventure/...). The effects
// draw a quad from 0 to 1 over their target; tex_scale maps it onto the part
// of the input texture the image covers.

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

static_assert(sizeof(PostProcessor::EffectUniforms) == 48, "EffectUniforms must match ADV_Effect");

PostProcessor::Effect::Effect(const char* vertexshader, const char* fragmentshader, const std::vector<const char*>& samplers) : mFade(false){
  mShader = Engine::instance()->getGraphics()->createShader(vertexshader, fragmentshader, samplers);
}

PostProcessor::Effect::~Effect(){
  Engine::instance()->getGraphics()->destroyShader(mShader);
}

void PostProcessor::Effect::init(const Vec2f& size){
  float powx = (float)Engine::roundToPowerOf2((unsigned)size.x);
  float powy = (float)Engine::roundToPowerOf2((unsigned)size.y);
  mUniforms.texScale[0] = size.x/powx;
  mUniforms.texScale[1] = size.y/powy;
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

void PostProcessor::Effect::draw(CGE::Gfx::Shader* shader, BlitObject* input, const std::vector<CGE::Gfx::Texture*>& textures){
  Graphics* gfx = Engine::instance()->getGraphics();
  gfx->clear();
  gfx->setTexture(input->getTexture());
  if (gfx->prepare(shader, Graphics::getVertexLayout(), CGE::Gfx::PrimitiveType::TriangleStrip, &mUniforms, sizeof(mUniforms))){
    CGE::Gfx::RenderDevice& device = gfx->getDevice();
    for (size_t i = 0; i < textures.size(); ++i)
      device.setTexture(CGE::Gfx::SLOT_TEXTURE+1+(int)i, textures[i], gfx->getSampler());
    device.setVertexBuffer(0, gfx->getQuadBuffer());
    device.draw(4);
  }
  gfx->setTexture(nullptr);
}

/*dark bloom, bloom, hell and whoosh*/

class BloomEffect : public PostProcessor::Effect{
public:
  BloomEffect(const char* name, const char* fragmentshader) : Effect("effect_blur.vert", fragmentshader){
    mName = name;
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mUniforms.pixelOffset[0] = 1.0f/size.x;
    mUniforms.pixelOffset[1] = 1.0f/size.y;
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
    mUniforms.strength = mInterpolator.current();
    draw(input);
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
  Interpolator mInterpolator;
  State mState;
};

/*noise*/

class NoiseEffect : public PostProcessor::Effect{
public:
  NoiseEffect(const char* name, const char* fragmentshader, int numChannels) : Effect("effect.vert", fragmentshader, {"blendtex"}), mBlendTex(0), mChannels(numChannels){
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
    Effect::init(size);
  }
  virtual void deinit(){
    if (mBlendTex != 0){
      delete mBlendTex;
      mBlendTex = nullptr;
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
    Engine::instance()->getGraphics()->updateTexture(mBlendTex, &mImage);
    return true;
  }
  virtual void apply(BlitObject* input){
    mUniforms.strength = mInterpolator.current();
    draw(input, {mBlendTex});
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
  Interpolator mInterpolator;
  bool mFadeout;
  CGE::Gfx::Texture* mBlendTex;
  CGE::Image mImage;
  int mChannels;
};

/*motion blur and warp speed*/

class MotionBlurEffect : public PostProcessor::Effect{
public:
  MotionBlurEffect(const char* name, const char* fragmentshader) : Effect("effect.vert", fragmentshader, {"motion1", "motion2", "motion3"}){
    mName = name;
    mStdShader = Engine::instance()->getGraphics()->createShader("effect.vert", "copy.frag");
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
    Engine::instance()->getGraphics()->destroyShader(mStdShader);
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
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
      mPrevFrames.front()->bind();
      draw(mStdShader.get(), input);
      mPrevFrames.front()->unbind();
    }
    mTakeCount++;

    if (mPrevFrames.size() < 3){
      draw(mStdShader.get(), input);
      return;
    }
    std::vector<CGE::Gfx::Texture*> frames;
    for (std::list<RenderableBlitObject*>::iterator iter = mPrevFrames.begin(); iter != mPrevFrames.end(); ++iter){
      frames.push_back((*iter)->getTexture());
    }
    draw(input, frames);
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
  std::unique_ptr<CGE::Gfx::Shader> mStdShader;
  std::list<RenderableBlitObject*> mPrevFrames;
  std::list<RenderableBlitObject*> mGenFrames;
  int mTakeFrame;
  int mTakeCount;
};

/*heat and underwater*/

class HeatEffect : public PostProcessor::Effect{
public:
  enum Type{
    RANDOM,
    SINE,
  };
  HeatEffect(const std::string& name, Type type) : Effect("effect.vert", "heat.frag", {"blendtex"}), mBlendTex(0), mTimeAccu(0), mLineCount(0){
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
    Effect::init(size);
    mUniforms.pixelOffset[0] = 1.0f/size.x;
    mUniforms.pixelOffset[1] = 1.0f/size.y;
  }
  virtual void deinit(){
    if (mBlendTex != 0){
      delete mBlendTex;
      mBlendTex = nullptr;
    }
  }
  virtual void activate(bool fade, ...){
    Effect::activate(fade);
    if (fade){
      memset(mImage.getData(), 127, mImage.getImageSize());
      Engine::instance()->getGraphics()->updateTexture(mBlendTex, &mImage);
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
      Engine::instance()->getGraphics()->updateTexture(mBlendTex, &mImage);
      mTimeAccu -= 50;
      if (mFadeout)
        mFadeoutPixels -= time;
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    draw(input, {mBlendTex});
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
  CGE::Gfx::Texture* mBlendTex;
  CGE::Image mImage;
  int mTimeAccu;
  int mFadeoutPixels;
  int mLineCount;
  Type mType;
};

/*drugged and zoom: the corners of the quad moved*/

class WarpEffect : public PostProcessor::Effect{
public:
  WarpEffect() : Effect("effect_warp.vert", "copy.frag"){
    CGE::Gfx::VertexAttribute offset;
    offset.location = 4;
    offset.format = CGE::Gfx::VertexFormat::Float2;
    offset.offset = offsetof(Vertex, ox);
    mLayout = Graphics::getVertexLayout();
    mLayout.attributes.push_back(offset);
    mLayout.buffers[0].stride = sizeof(Vertex);
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    CGE::Gfx::BufferDesc desc;
    desc.usage = CGE::Gfx::Usage::Stream;
    desc.size = 4*sizeof(Vertex);
    mVB = Engine::instance()->getGraphics()->getDevice().createBuffer(desc);
  }
  virtual void deinit(){
    mVB.reset();
    Effect::deinit();
  }
protected:
  struct Vertex{
    Graphics::Vertex vertex;
    float ox, oy;
  };
  //! draws the input with the corners of the quad moved by the offsets (x, y per corner)
  void drawWarped(BlitObject* input, const float offsets[8]){
    Graphics* gfx = Engine::instance()->getGraphics();
    gfx->clear();
    if (!mVB)
      return;
    Graphics::Vertex quad[4];
    Graphics::getQuad(quad);
    Vertex verts[4];
    for (int i = 0; i < 4; ++i)
      verts[i] = {quad[i], offsets[2*i], offsets[2*i+1]};
    CGE::Gfx::RenderDevice& device = gfx->getDevice();
    device.updateBuffer(mVB.get(), 0, sizeof(verts), verts);
    gfx->setTexture(input->getTexture());
    if (gfx->prepare(mShader.get(), mLayout, CGE::Gfx::PrimitiveType::TriangleStrip, &mUniforms, sizeof(mUniforms))){
      device.setVertexBuffer(0, mVB.get());
      device.draw(4);
    }
    gfx->setTexture(nullptr);
  }
  std::unique_ptr<CGE::Gfx::Buffer> mVB;
  CGE::Gfx::VertexLayout mLayout;
};

class DruggedEffect : public WarpEffect{
public:
  DruggedEffect(){
    mName = "drugged";
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
    float tmp[8];
    for (int i = 0; i < 8; ++i){
      tmp[i] = mVerts[i].current();
      if (i == 0 || i == 2 || i == 3 || i == 7)
        tmp[i] *= -1;
    }
    drawWarped(input, tmp);
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
  float mStrength;
};

/*lightning*/

class LightningEffect : public PostProcessor::Effect{
public:
  LightningEffect() : Effect("effect.vert", "lightning.frag", {"blendtex"}), mFBO(nullptr){
    mName = "lightning";
    mDrawShader = Engine::instance()->getGraphics()->createShader("lightning_draw.vert", "lightning_draw.frag");
    CGE::Gfx::VertexAttribute position;
    position.location = CGE::Gfx::ATTR_POSITION;
    position.format = CGE::Gfx::VertexFormat::Float3;
    position.offset = offsetof(LineVertex, x);
    CGE::Gfx::VertexAttribute color;
    color.location = CGE::Gfx::ATTR_COLOR;
    color.format = CGE::Gfx::VertexFormat::Float4;
    color.offset = offsetof(LineVertex, r);
    mLineLayout.attributes = {position, color};
    CGE::Gfx::VertexBufferLayout buffer;
    buffer.stride = sizeof(LineVertex);
    mLineLayout.buffers = {buffer};
  }
  ~LightningEffect(){
    deactivate();
    Engine::instance()->getGraphics()->destroyShader(mDrawShader);
  }
  virtual void init(const Vec2f& size){
    mFBO = new RenderableBlitObject((int)size.x, (int)size.y, 0);
    mFBO->realize();
    Effect::init(size);
  }
  virtual void deinit(){
    delete mFBO;
    mFBO = nullptr;
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
    ltn.timeaccu2 = (int)(ltn.delay*1.5f);
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
    Graphics* gfx = Engine::instance()->getGraphics();
    mFBO->bind();
    gfx->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    gfx->clear();
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
      drawLines(iter->second->verts.get(), numVerts);
      drawLines(iter->second->verts2.get(), numVerts);
    }
    mFBO->unbind();

    return true;
  }
  virtual void apply(BlitObject* input){
    draw(input, {mFBO->getTexture()});
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
  struct LineVertex{
    float x, y, z;
    float r, g, b, a;
  };
  struct Lightning{
    Vec2f start;
    Vec2f end;
    Color color;
    int numSpikes;
    float height;
    int delay;

    int timeaccu;
    std::unique_ptr<CGE::Gfx::Buffer> verts;
    int timeaccu2;
    std::unique_ptr<CGE::Gfx::Buffer> verts2;

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
      verts.reset();
      timeaccu2 = (int)(delay*1.5f);
      verts2.reset();
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
  void drawLines(CGE::Gfx::Buffer* verts, int numVerts){
    if (!verts)
      return;
    Graphics* gfx = Engine::instance()->getGraphics();
    if (gfx->prepare(mDrawShader.get(), mLineLayout, CGE::Gfx::PrimitiveType::LineStrip, nullptr, 0)){
      gfx->getDevice().setVertexBuffer(0, verts);
      gfx->getDevice().draw(numVerts);
    }
  }
  void drawFlash(int numVerts, Lightning* ltn, bool secondFlash){
    std::vector<LineVertex> vb(numVerts);
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
      float opacity = 1.0f;
      if (secondFlash)
        opacity = 0.75f;
      vb[i + 1] = {vert.x + ortho.x*spikeheight, vert.y + ortho.y*spikeheight, 0,
        ltn->color.r / 255.0f, ltn->color.g / 255.0f, ltn->color.b / 255.0f, ltn->color.a / 255.0f * opacity};
    }
    delete [] knots;
    float opacity = ltn->color.a / 255.0f;
    if (secondFlash)
      opacity = 0.25f;
    vb[0] = {ltn->start.x, ltn->start.y, 0, ltn->color.r / 255.0f, ltn->color.g / 255.0f, ltn->color.b / 255.0f, opacity};
    vb[numVerts-1] = {ltn->end.x, ltn->end.y, 0, ltn->color.r / 255.0f, ltn->color.g / 255.0f, ltn->color.b / 255.0f, opacity};
    CGE::Gfx::BufferDesc desc;
    desc.size = numVerts*sizeof(LineVertex);
    std::unique_ptr<CGE::Gfx::Buffer> buffer = Engine::instance()->getGraphics()->getDevice().createBuffer(desc, vb.data());
    if (secondFlash)
      ltn->verts2 = std::move(buffer);
    else
      ltn->verts = std::move(buffer);
  }
  RenderableBlitObject* mFBO;
  std::unique_ptr<CGE::Gfx::Shader> mDrawShader;
  CGE::Gfx::VertexLayout mLineLayout;
  std::map<int,Lightning*> mLigthnings;
};

/*fog*/

class FogEffect : public PostProcessor::Effect{
public:
  FogEffect() : Effect("effect.vert", "fog.frag"){
    mName = "fog";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mUniforms.blendColor[0] = mUniforms.blendColor[1] = mUniforms.blendColor[2] = 0.5f;
    mUniforms.blendColor[3] = 1.0f;
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
    mUniforms.strength = mInterpolator.current();
    draw(input);
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
  Interpolator mInterpolator;
  bool mFadeout;
};

/*zoom*/

class ZoomEffect : public WarpEffect{
public:
  ZoomEffect(){
    mName = "zoom";
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
    float tmp[8];
    for (int i = 0; i < 8; ++i){
      tmp[i] = mVerts[i].current();
      if (i == 0 || i == 2 || i == 3 || i == 7)
        tmp[i] *= -1;
    }
    drawWarped(input, tmp);
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
    if (Engine::instance()->getGraphics()->getDevice().getClipSpace().originTopLeft)
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
  Interpolator mVerts[8];
  Vec2i mPos;
  float mScale;
  int mFadetime;
};

/*white flash*/

class FlashEffect : public PostProcessor::Effect{
public:
  FlashEffect() : Effect("effect.vert", "fog.frag"){
    mName = "whiteflash";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mUniforms.blendColor[0] = mUniforms.blendColor[1] = mUniforms.blendColor[2] = mUniforms.blendColor[3] = 1.0f;
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
    mUniforms.strength = mInterpolator.current();
    draw(input);
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
  Interpolator mInterpolator;
  bool mFadeout;
  int mFadeoutTime;
};

/*pixelate*/

class PixelateEffect : public PostProcessor::Effect{
public:
  PixelateEffect() : Effect("effect.vert", "pixelate.frag"){
    mName = "pixelate";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mUniforms.pixelOffset[0] = 1.0f/Engine::roundToPowerOf2((unsigned)size.x);
    mUniforms.pixelOffset[1] = 1.0f/Engine::roundToPowerOf2((unsigned)size.y);
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    mPixSize = (float)va_arg(args, double);
    va_end(args);
    Effect::activate(fade);
  }
  virtual void apply(BlitObject* input){
    Vec2i camOffset = -Engine::instance()->getRoom("")->getScrollOffset();
    mUniforms.strength = mPixSize;
    mUniforms.camOffset[0] = fmodf((float)camOffset.x, mPixSize);
    mUniforms.camOffset[1] = fmodf((float)camOffset.y, mPixSize);
    draw(input);
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
  float mPixSize;
};

/*scanlines*/

class ScanlineEffect : public PostProcessor::Effect{
public:
  ScanlineEffect() : Effect("effect.vert", "scanlines.frag"){
    mName = "scanlines";
  }
  virtual void init(const Vec2f& size){
    Effect::init(size);
    mUniforms.pixelOffset[0] = 1.0f/Engine::roundToPowerOf2((unsigned)size.x);
    mUniforms.pixelOffset[1] = 1.0f/Engine::roundToPowerOf2((unsigned)size.y);
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
    mUniforms.strength = mInterpolator.current();
    draw(input);
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
  Interpolator mInterpolator;
  bool mFadeout;
};


/* Postprocessor */

#define REGISTER_EFFECT(effect, class, ...) class* effect = new class(__VA_ARGS__); mEffects[effect->getName()] = effect;

PostProcessor::PostProcessor(int width, int height, int depth) : mResult1(width, height, depth), mResult2(width, height, depth){
  mResult1.realize();
  mResult2.realize();
  REGISTER_EFFECT(darkbloom, BloomEffect, "darkbloom", "darkbloom.frag");
  REGISTER_EFFECT(noise, NoiseEffect, "noise", "noise.frag", 1);
  REGISTER_EFFECT(hell, BloomEffect, "hell", "hell.frag");
  REGISTER_EFFECT(motionblur, MotionBlurEffect, "motionblur", "motionblur.frag");
  REGISTER_EFFECT(heat, HeatEffect, "heat", HeatEffect::RANDOM);
  REGISTER_EFFECT(whoosh, BloomEffect, "whoosh", "whoosh.frag");
  REGISTER_EFFECT(bloom, BloomEffect, "bloom", "bloom.frag")
  REGISTER_EFFECT(drugged, DruggedEffect);
  REGISTER_EFFECT(lightning, LightningEffect);
  REGISTER_EFFECT(fog, FogEffect);
  REGISTER_EFFECT(zoom, ZoomEffect);
  REGISTER_EFFECT(flash, FlashEffect);
  REGISTER_EFFECT(underwater, HeatEffect, "underwater", HeatEffect::SINE);
  REGISTER_EFFECT(colornoise, NoiseEffect, "colornoise", "colornoise.frag", 4);
  REGISTER_EFFECT(pixelate, PixelateEffect);
  REGISTER_EFFECT(scanlines, ScanlineEffect);
  REGISTER_EFFECT(warpspeed, MotionBlurEffect, "warpspeed", "motionblur.frag");
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
  return nullptr;
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
