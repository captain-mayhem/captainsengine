#include "Forward.h"
#include <renderer/scene.h>

using namespace CGE;

void ForwardRenderer::render(){
  if (!mScene)
    return;
  mScene->render();
}