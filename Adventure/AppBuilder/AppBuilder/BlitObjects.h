#ifndef BLIT_OBJECTS_H
#define BLIT_OBJECTS_H

#ifdef WIN32
#include <Windows.h>
#endif
#include <GL/glew.h>

#include <string>

#include "Vector.h"

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
protected:
  Vec2i mPos;
  Vec2i mSize;
  int mDepth;
};

class BlitObject : public BaseBlitObject{
public:
  BlitObject(int width, int height);
  BlitObject(std::string texture, int depth, Vec2i offset);
  BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset);
  virtual ~BlitObject();
  bool operator<(const BlitObject& obj);
  void render(Vec2i pos, bool mirrorx);
  virtual void blit();
protected:
  Vec2i mOffset;
  Vec2f mScale;
  GLuint mTex;
  bool mMirrorX;
  bool mDeleteTex;
};

class LightingBlitObject : public BaseBlitObject{
public:
  LightingBlitObject(int depth, const Vec2i& size);
  virtual ~LightingBlitObject();
  virtual void blit();
  void render(const Vec2i& pos);
  void setColor(const Color& col) {mColor = col;}
  Color getColor() {return mColor;}
protected:
  Color mColor;
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
protected:
  GLuint mRenderBuffer;
  GLuint mFrameBuffer;
};

#endif
