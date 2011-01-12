#ifndef BLIT_OBJECTS_H
#define BLIT_OBJECTS_H

#include <string>

#include "Vector.h"
#include "Renderer.h"

class Inventory;
class InventoryDisplay;
class Animation;

class AnimationEndHandler{
public:
  virtual bool animationEnded(Animation* anim)=0;
};

struct Color{
  Color(){
    r = 255; g=255; b=255; a=255;
  }
  Color(unsigned packedcolor){
    r = (packedcolor >> 0) & 0xFF;
    g = (packedcolor >> 8) & 0xFF;
    b = (packedcolor >> 16) & 0xFF;
    a = 255;
  }
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

class BaseBlitObject{
public:
  BaseBlitObject(int depth, const Vec2i& size);
  virtual ~BaseBlitObject();
  virtual void blit()=0;
  int getDepth() {return mDepth;}
  void setDepth(int depth) {mDepth = depth;}
  void setColor(const Color& col) {mColor = col;}
  Color getColor() {return mColor;}
protected:
  Vec2i mPos;
  Vec2i mSize;
  int mDepth;
  Color mColor;
};

class BlitObject : public BaseBlitObject{
public:
  BlitObject(int width, int height);
  BlitObject(std::string texture, int depth, Vec2i offset);
  BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset);
  virtual ~BlitObject();
  bool operator<(const BlitObject& obj);
  void render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize);
  virtual void blit();
  Vec2i getOffset() {return mOffset;}
  void setMirrorOffset(const Vec2i& mirror) {mMirrorOffset = mirror;}
protected:
  Vec2i mOffset;
  Vec2f mScale;
  GLuint mTex;
  Vec2f mZoomScale;
  bool mDeleteTex;
  Vec2i mMirrorOffset;
};

class LightingBlitObject : public BaseBlitObject{
public:
  LightingBlitObject(int depth, const Vec2i& size);
  virtual ~LightingBlitObject();
  virtual void blit();
  void render(const Vec2i& pos);
};

class ScrollBlitObject : public BaseBlitObject{
public:
  ScrollBlitObject(int depth);
  virtual ~ScrollBlitObject();
  virtual void blit();
  void render(const Vec2i& pos);
};

class RenderableBlitObject : public BlitObject{
public:
  RenderableBlitObject(int width, int height);
  ~RenderableBlitObject();
  void bind();
  static void unbind();
protected:
  GLuint mRenderBuffer;
  GLuint mFrameBuffer;
};

class DynamicAnimation{
public:
  DynamicAnimation();
  virtual ~DynamicAnimation();
  virtual void update(unsigned interval)=0;
};

class CircleScreenChange : public RenderableBlitObject, public DynamicAnimation{
public:
  CircleScreenChange(int width, int height, int duration);
  virtual ~CircleScreenChange();
  virtual void update(unsigned interval);
};

#endif
