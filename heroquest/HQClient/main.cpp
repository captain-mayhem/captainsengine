#include <iostream>
#include "system/engine.h"
#include "renderer/renderer.h"

#include "renderer.h"

using namespace System;
using namespace Graphics;

void engineMain(int argc, char** argv){
  Renderer* rend = Engine::instance()->getRenderer();
  HQRenderer::init(rend);

  rend->setInitCB(HQRenderer::initialize);
  rend->setRenderCB(HQRenderer::paint);
}