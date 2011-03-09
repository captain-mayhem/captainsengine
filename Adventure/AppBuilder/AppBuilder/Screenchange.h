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
  CircleScreenChange(int width, int height, int depth, int duration);
  virtual ~CircleScreenChange();
  virtual bool update(unsigned interval);
protected:
  void generateCircle(float radius, int segments);
  int mDuration;
  int mCurrentTime;
  float* mVerts;
  bool mClosing;
  Screenshot mShot;
};

#endif
