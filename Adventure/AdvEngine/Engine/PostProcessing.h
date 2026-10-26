#ifndef ADV_POSTPROCESSING_H
#define ADV_POSTPROCESSING_H

#include <map>
#include <list>
#include <memory>
#include <vector>

#include "BlitObjects.h"

namespace CGE{
namespace Gfx{
class Shader;
}
}

namespace adv{

class PostProcessor{
public:
  //! the ADV_Effect uniform block of the effect shaders (shaders/effect.glsl)
  struct EffectUniforms{
    float texScale[2] = {1, 1};
    float pixelOffset[2] = {0, 0};
    float blendColor[4] = {0, 0, 0, 0};
    float camOffset[2] = {0, 0};
    float strength = 0;
    float padding = 0;
  };

  class Effect{
  public:
    //! an effect drawn with a vertex and a fragment shader of the Adventure's shaders
    /*! samplers: the fragment shader's samplers after tex, in slots 1, 2, ... */
    Effect(const char* vertexshader, const char* fragmentshader, const std::vector<const char*>& samplers = std::vector<const char*>());
    virtual ~Effect();
    virtual void init(const Vec2f& size);
    virtual void deinit() {}
    virtual bool update(unsigned time) {return true;}
    virtual void apply(BlitObject* input)=0;
    //! fade is a bool; an int, since va_start after a parameter that is promoted is undefined
    virtual void activate(int fade, ...);
    void activate() {Effect::activate(mFade);}
    virtual void deactivate();
    virtual void deactivate(int data);
    const std::string& getName() {return mName;}
    virtual std::ostream& save(std::ostream& out);
    virtual std::istream& load(std::istream& in);
  protected:
    //! clears the target and draws the input with shader over it
    /*! textures go into the slots 1, 2, ... */
    void draw(CGE::Gfx::Shader* shader, BlitObject* input, const std::vector<CGE::Gfx::Texture*>& textures = std::vector<CGE::Gfx::Texture*>());
    void draw(BlitObject* input, const std::vector<CGE::Gfx::Texture*>& textures = std::vector<CGE::Gfx::Texture*>()) {draw(mShader.get(), input, textures);}
    std::unique_ptr<CGE::Gfx::Shader> mShader;
    EffectUniforms mUniforms;
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
