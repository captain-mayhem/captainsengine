#ifndef SCREENCHANGE_H
#define SCREENCHANGE_H

#include "BlitObjects.h"

namespace adv{

class ScreenChangeBase : public RenderableBlitObject, public DynamicAnimation{
public:
  ScreenChangeBase(int width, int height, int depth) : RenderableBlitObject(width, height, depth), mUnload(false) {}
  virtual Object2D* getTarget() {return NULL;}
  virtual Type getType() {return SCREEN;}
  virtual void finish() {}
  void setUnload(bool unload) {mUnload = unload;}
  void screenChangeSwitch();
protected:
  bool mUnload;
};

class Screenshot : public RenderableBlitObject{
public:
  Screenshot(int depth);
  void take();
};

class CircleScreenChange : public ScreenChangeBase{
public:
  CircleScreenChange(int width, int height, int depth, int duration, int segments);
  virtual ~CircleScreenChange();
  virtual bool update(unsigned interval);
protected:
  void generateCircle(float radius);
  int mDuration;
  int mCurrentTime;
  float* mVerts;
  bool mClosing;
  Screenshot mShot;
  int mSegments;
};

class BlendScreenChange : public ScreenChangeBase{
public:
  BlendScreenChange(int width, int height, int depth, int duration);
  virtual bool update(unsigned interval);
protected:
  int mDuration;
  int mCurrentTime;
  Screenshot mShot;
};

class FadeoutScreenChange : public ScreenChangeBase{
public:
  FadeoutScreenChange(int width, int height, int depth, int duration);
  virtual bool update(unsigned interval);
protected:
  int mDuration;
  int mCurrentTime;
  Screenshot mShot;
  bool mClosing;
};

class ShuttersScreenChange : public ScreenChangeBase{
public:
  ShuttersScreenChange(int width, int height, int depth, int duration);
  virtual bool update(unsigned interval);
protected:
  int mDuration;
  int mCurrentTime;
  Screenshot mShot;
  bool mClosing;
};

class ClockScreenChange : public ScreenChangeBase{
public:
  ClockScreenChange(int width, int height, int depth, int duration, int segments);
  virtual ~ClockScreenChange();
  virtual bool update(unsigned interval);
protected:
  void generateCircle(float radius);
  int mDuration;
  int mCurrentTime;
  float* mVerts;
  bool mClosing;
  Screenshot mShot;
  int mSegments;
};

}

#endif
