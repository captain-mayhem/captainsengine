#include "PostProcessing.h"

#include "Renderer.h"
#include "Engine.h"

using namespace adv;

class Interpolator{
public:
  Interpolator() : mSource(0), mTarget(0), mCurrent(0), mTime(0), mTimeAccu(0){}
  Interpolator(float from, float to, int time) :
  mSource(from), mTarget(to), mCurrent(from), mTime((float)time), mTimeAccu(0){
  }
  void set(float from, float to, int time){
    mSource = from;
    mTarget = to;
    mCurrent = from;
    mTime = (float)time;
    mTimeAccu = 0;
  }
  bool update(int time){
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
  void reset() {mCurrent = mSource; mTimeAccu = 0;}
private:
  float mSource;
  float mTarget;
  float mCurrent;
  float mTime;
  int mTimeAccu;
};

PostProcessor::Effect::Effect(const char* vertexsource, const char* fragmentsource){
  mShader.addShader(GL_VERTEX_SHADER, vertexsource);
  mShader.addShader(GL_FRAGMENT_SHADER, fragmentsource);
}

PostProcessor::Effect::~Effect(){
}

void PostProcessor::Effect::activate(bool fade, ...){
  Engine::instance()->getPostProcessor()->mActiveEffects.insert(std::make_pair("bla", this));
}

static const char vertexshadersource[] =
"precision mediump float;\n"
"attribute vec3 position;\n"
"attribute vec4 texCoord;\n"
"\n"
"uniform mat4 matrices[3];\n"
"\n"
"varying vec2 tex_coord;\n"
"varying vec2 tex_coord2;\n"
"\n"
"void main(){\n"
"  tex_coord = vec2(position.x*0.625, (0.0+position.y)*0.9375);\n"
"  tex_coord2 = vec2(position.x*2.0-1.0, (0.0+position.y)*2.0-1.0)*vec2(0.0015625*2, 0.002083*2);\n"
"  gl_Position = vec4(position.x*2.0-1.0, position.y*2.0-1.0, 0.0, 1.0);\n"
"}\n"
"";

static const char fragmentshadersource[] =
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
  DarkBloomEffect() : Effect(vertexshadersource, fragmentshadersource){
  }
  virtual void init(){
    mShader.linkShaders();
    mShader.activate();
    GLint tex = mShader.getUniformLocation("texture");
    mShader.uniform(tex, 0);
    mIntensityLoc = mShader.getUniformLocation("strength");
    mShader.deactivate();
  }
  virtual void activate(bool fade, ...){
    va_list args;
    va_start(args, fade);
    float strength = (float)va_arg(args,double);
    bool animate = va_arg(args,bool);
    va_end(args);
    mInterpolator.set(0, strength, 5000);
    Effect::activate(fade);
  }
  virtual bool update(unsigned time) {
    if (!mInterpolator.update(time)){
      mInterpolator.set(mInterpolator.current(), mInterpolator.source(), (int)mInterpolator.time());
      mInterpolator.reset();
    }
    return true;
  }
  virtual void apply(BlitObject* input){
    glBindTexture(GL_TEXTURE_2D, input->getTexture());
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mShader.activate();
    mShader.uniform(mIntensityLoc, mInterpolator.current());
    //GLint pos = mEffects["darkbloom"]->getShader().getAttribLocation("position");
    //glVertexAttribPointer(pos, size, type, GL_FALSE, 0, Engine::instance()->getVertices());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    mShader.deactivate();
  }
private:
  GLint mIntensityLoc;
  Interpolator mInterpolator;
};



PostProcessor::PostProcessor(int width, int height, int depth) : mResult(width, height, depth){
  DarkBloomEffect* darkbloom = new DarkBloomEffect();
  mEffects["darkbloom"] = darkbloom;
}

PostProcessor::~PostProcessor(){
  for (std::map<std::string,Effect*>::iterator iter = mEffects.begin(); iter != mEffects.end(); ++iter){
    delete iter->second;
  }
}

BlitObject* PostProcessor::process(BlitObject* input, unsigned time){
  static bool first = true;
  if (first){
    mEffects["darkbloom"]->init();
    mEffects["darkbloom"]->activate(true, 1.0f, true);
    first = false;
  }
  mResult.bind();
  mEffects["darkbloom"]->update(time);
  mEffects["darkbloom"]->apply(input);
  mResult.unbind();
  //return &mResult;
  return input;
}

PostProcessor::Effect* PostProcessor::getEffect(const std::string& effect){
  std::map<std::string,Effect*>::iterator iter = mActiveEffects.find(effect);
  if (iter != mActiveEffects.end())
    return iter->second;
  iter = mEffects.find(effect);
  if (iter != mEffects.end()){
    return iter->second;
  }
  return NULL;
}
