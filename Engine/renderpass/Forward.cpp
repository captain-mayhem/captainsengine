#include "Forward.h"
#include <renderer/scene.h>
#include <renderer/renderer.h>
#include <renderer/camera.h>
#include <system/engine.h>

using namespace CGE;

static Vec3f camPos;

static int compareLightDist(const void * a, const void * b){
  Light* l1 = *(Light**)a;
  Light* l2 = *(Light**)b;
  float l1dist = (Vec3f(l1->getPosition()) - camPos).length();
  float l2dist = (Vec3f(l2->getPosition()) - camPos).length();
  return (int)((l1dist - l2dist) * 100);
}

void ForwardRenderer::render(Mesh::DrawPart part){
  if (!mScene)
    return;
  Renderer* rend = Engine::instance()->getRenderer();
  rend->enableLighting(true);
  std::vector<Light*> lights = mScene->getLights();
  unsigned numLights = lights.size();
  if (numLights > 8){
    numLights = 8;
    if (mScene->getActiveCam()){
      camPos = mScene->getActiveCam()->getPosition();
      qsort(lights.data(), lights.size(), sizeof(Light*), compareLightDist);
    }
  }
  rend->setNumLights(numLights);
  for (unsigned i = 0; i < numLights; ++i){
    rend->setLight(i, *lights[i]);
  }
  mScene->render(part);
  rend->enableLighting(false);
}