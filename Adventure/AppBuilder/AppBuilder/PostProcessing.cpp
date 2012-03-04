#include "PostProcessing.h"

using namespace adv;

PostProcessor::PostProcessor(int width, int height, int depth) : mResult(width, height, depth){

}

BlitObject* PostProcessor::process(BlitObject* input){
  return input;
}
