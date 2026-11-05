#ifndef SCREENCHANGE_H
#define SCREENCHANGE_H

#include <memory>

#include <renderer/screentransition.h>

#include "BlitObjects.h"

namespace adv{

//! a screen change (Ids.h) on the engine's screen transitions
/*! An animation of the Animator that draws itself over the game at its
    depth. Its first update copies the screen, which still shows the last
    frame with the old room, as the picture the closing half is drawn over;
    the rooms have already changed below it. */
class ScreenChangeAnimation : public BaseBlitObject, public DynamicAnimation{
public:
  //! change mustn't be SC_DIRECT
  ScreenChangeAnimation(ScreenChange change, int depth);
  virtual ~ScreenChangeAnimation();
  virtual Object2D* getTarget() {return NULL;}
  virtual Type getType() {return SCREEN;}
  virtual void finish() {}
  virtual bool update(unsigned interval);
  virtual void blit();
  //! the room was unloaded without another being loaded (unloadroom)
  void setUnload(bool unload) {mUnload = unload;}
  //! the transition and its milliseconds of a screen change
  static CGE::Gfx::TransitionKind getKind(ScreenChange change, int* duration);
protected:
  CGE::Gfx::ScreenTransition mTransition;
  std::unique_ptr<CGE::Gfx::ScreenTransitionRenderer> mRenderer;
  bool mStarted;
  bool mUnload;
};

}

#endif
