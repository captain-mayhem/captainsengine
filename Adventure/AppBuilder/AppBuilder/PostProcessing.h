#ifndef ADV_POSTPROCESSING_H
#define ADV_POSTPROCESSING_H

#include "BlitObjects.h"

namespace adv{

class PostProcessor{
public:
  PostProcessor(int width, int height, int depth);
  BlitObject* process(BlitObject* input);
private:
  RenderableBlitObject mResult;
};

}

#endif
