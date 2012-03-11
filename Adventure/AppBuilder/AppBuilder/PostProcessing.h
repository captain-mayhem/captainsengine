#ifndef ADV_POSTPROCESSING_H
#define ADV_POSTPROCESSING_H

#include "BlitObjects.h"
#include "Renderer.h"

namespace adv{

class PostProcessor{
public:
  class Effect{
  public:
    Effect(const char* vertexsource, const char* fragmentsource);
    virtual ~Effect();
    virtual void init() {mShader.linkShaders();}
    virtual void deinit() {mShader.deleteShaders();}
    virtual bool update(unsigned time) {return true;}
    virtual void apply(BlitObject* input)=0;
    virtual void activate(bool fade, ...);
    virtual void deactivate();
    const std::string& getName() {return mName;}
  protected:
    GL2Shader mShader;
    std::string mName;
    bool mFade;
  };
  
  PostProcessor(int width, int height, int depth);
  ~PostProcessor();
  BlitObject* process(BlitObject* input, unsigned time);
  Effect* getEffect(const std::string& effect);
  void stopEffects();

  friend class Effect;
private:
  RenderableBlitObject mResult;
  std::map<std::string, Effect*> mEffects;
  std::list<Effect*> mActiveEffects;
};

}

#endif
