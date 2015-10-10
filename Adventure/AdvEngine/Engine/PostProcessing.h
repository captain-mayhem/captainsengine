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
    virtual void init(const Vec2f& size) {}
    virtual void deinit() {}
    virtual bool update(unsigned time) {return true;}
    virtual void apply(BlitObject* input)=0;
    virtual void activate(bool fade, ...);
    void activate() {Effect::activate(mFade);}
    virtual void deactivate();
    virtual void deactivate(int data);
    const std::string& getName() {return mName;}
    virtual std::ostream& save(std::ostream& out);
    virtual std::istream& load(std::istream& in);
  protected:
    CGE::Shader& mShader;
    std::string mName;
    bool mFade;
  };
  
  PostProcessor(int width, int height, int depth);
  ~PostProcessor();
  BlitObject* process(BlitObject* input, unsigned time);
  Effect* getEffect(const std::string& effect);
  void stopEffects();
  std::ostream& save(std::ostream& out);
  std::istream& load(std::istream& in);

  friend class Effect;
private:
  RenderableBlitObject mResult1;
  RenderableBlitObject mResult2;
  std::map<std::string, Effect*> mEffects;
  std::list<Effect*> mActiveEffects;
};

}

#endif
