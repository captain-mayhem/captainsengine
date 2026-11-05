#include "Screenchange.h"
#include "Engine.h"
#include "Graphics.h"
#include <renderer/renderdevice.h>
#include <system/engine.h>

using namespace adv;
using namespace CGE::Gfx;

TransitionKind ScreenChangeAnimation::getKind(ScreenChange change, int* duration){
  *duration = 2000;
  switch (change){
    case SC_DIRECT:
      *duration = 0;
      return TransitionKind::Cut;
    case SC_FADEOUT:
      return TransitionKind::FadeOut;
    case SC_RECTANGLE:
      return TransitionKind::Rectangle;
    case SC_CIRCLE:
      return TransitionKind::Circle;
    case SC_SHUTTERS:
      return TransitionKind::Shutters;
    case SC_CLOCK:
      return TransitionKind::Clock;
    case SC_BLEND:
      *duration = 1000;
      return TransitionKind::Blend;
    case SC_BLEND_SLOW:
      return TransitionKind::Blend;
  }
  *duration = 0;
  return TransitionKind::Cut;
}

ScreenChangeAnimation::ScreenChangeAnimation(ScreenChange change, int depth) :
BaseBlitObject(depth, Engine::instance()->getResolution()), mStarted(false), mUnload(false){
  int duration = 0;
  TransitionKind kind = getKind(change, &duration);
  mTransition = ScreenTransition(kind, duration/1000.0);
}

ScreenChangeAnimation::~ScreenChangeAnimation(){
  Engine::instance()->getCursor()->showLoading(false);
}

bool ScreenChangeAnimation::update(unsigned interval){
  if (!mStarted){
    mStarted = true;
    //before this frame's clear the screen shows the last frame, with the old room
    Graphics* gfx = Engine::instance()->getGraphics();
    RenderDevice& device = gfx->getDevice();
    mRenderer = std::make_unique<ScreenTransitionRenderer>(device);
    gfx->copyScreen(mRenderer->getPictureTarget(device.getWidth(), device.getHeight()));
    if (mUnload)
      Engine::instance()->forceNotToRenderUnloadingRoom();
  }
  else
    mTransition.advance(interval/1000.0);
  //the rooms changed already
  if (mTransition.isChangeDue())
    mTransition.changed();
  if (mTransition.isDone())
    return false;
  Engine::instance()->insertToBlit(this);
  return true;
}

void ScreenChangeAnimation::blit(){
  if (!mRenderer)
    return;
  //over the whole visible area, as the screen's copy
  CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
  CGE::Vec2f min = screen.getVisibleMin();
  CGE::Vec2f max = screen.getVisibleMax();
  ScreenArea area;
  area.x = min.x;
  area.y = min.y;
  area.width = max.x - min.x;
  area.height = max.y - min.y;
  TransitionPicture picture = mRenderer->getPicture();
  mRenderer->draw(mTransition, Engine::instance()->getGraphics()->getSpriteBatch(), area, picture.texture ? &picture : nullptr);
}
