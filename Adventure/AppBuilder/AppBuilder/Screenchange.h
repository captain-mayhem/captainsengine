#ifndef SCREENCHANGE_H
#define SCREENCHANGE_H

#include "BlitObjects.h"

class Screenshot : public RenderableBlitObject{
public:
  Screenshot(int depth);
  void take();
};

class CircleScreenChange : public RenderableBlitObject, public DynamicAnimation{
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

class BlendScreenChange : public RenderableBlitObject, public DynamicAnimation{
public:
  BlendScreenChange(int width, int height, int depth, int duration);
  virtual bool update(unsigned interval);
protected:
  int mDuration;
  int mCurrentTime;
  Screenshot mShot;
};

class FadeoutScreenChange : public RenderableBlitObject, public DynamicAnimation{
public:
  FadeoutScreenChange(int width, int height, int depth, int duration);
  virtual bool update(unsigned interval);
protected:
  int mDuration;
  int mCurrentTime;
  Screenshot mShot;
  bool mClosing;
};

#endif
