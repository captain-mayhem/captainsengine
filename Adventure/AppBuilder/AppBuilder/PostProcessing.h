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
    ~Effect();
    void activate() {mShader.activate();}
    void deactivate() {mShader.deactivate();}
    GL2Shader& getShader() {return mShader;}
  protected:
    GL2Shader mShader;
  };
  
  PostProcessor(int width, int height, int depth);
  ~PostProcessor();
  BlitObject* process(BlitObject* input);
private:
  RenderableBlitObject mResult;
  std::map<std::string, Effect*> mEffects;
};

}

#endif
