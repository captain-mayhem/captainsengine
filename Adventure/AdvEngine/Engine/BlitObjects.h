#ifndef BLIT_OBJECTS_H
#define BLIT_OBJECTS_H

#include <string>

#include <renderer/renderer.h>

#include "Vector.h"
#include "Ids.h"

namespace CGE{
  class Image;
}

namespace adv{

class Inventory;
class InventoryDisplay;
class Animation;

class AnimationEndHandler{
public:
  virtual bool animationEnded(Animation* anim)=0;
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
  enum BlendMode{
    BLEND_ALPHA,
    BLEND_ADDITIVE,
    BLEND_PREMULT_ALPHA,
  };
  BlitObject(int width, int height, int depth);
  BlitObject(std::string texture, const Vec2i& size, int depth, Vec2i offset);
  BlitObject(CGE::Texture* texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset);
  virtual ~BlitObject();
  BlitObject* clone();
  bool operator<(const BlitObject& obj);
  void render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize);
  virtual void blit();
  Vec2i getOffset() {return mOffset;}
  void setMirrorOffset(const Vec2i& mirror) {mMirrorOffset = mirror;}
  void setRotation(float angle) {mRotAngle = angle;}
  void setBlendMode(BlendMode mode) {mBlendMode = mode;}
  void updateTexture(unsigned width, unsigned height, void* data);
  CGE::Texture* getTexture() {return mTex;}
  virtual void realize();
  void realizeEmpty(CGE::Texture::Format format=CGE::Texture::RGBA);
protected:
  Vec2i mOffset;
  Vec2f mScale;
  CGE::Texture* mTex;
  CGE::Image* mImage;
  Vec2f mZoomScale;
  bool mDeleteTex;
  Vec2i mMirrorOffset;
  float mRotAngle;
  BlendMode mBlendMode;
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
  RenderableBlitObject(int width, int height, int depth);
  ~RenderableBlitObject();
  void bind();
  void unbind();
  virtual void realize();
protected:
  CGE::RenderTarget* mFrameBuffer;
  int mOldViewport[4];
};

class Object2D;

class DynamicAnimation{
public:
  enum Type{
    COLOR,
    SCALE,
    SCREEN,
    TIME,
    OPACITY,
  };
  DynamicAnimation();
  virtual ~DynamicAnimation();
  virtual bool update(unsigned interval)=0;
  virtual Object2D* getTarget()=0;
  virtual Type getType()=0;
  virtual void finish()=0;
};

class RoomObject;
class CharacterObject;

class MirrorObject : public RenderableBlitObject{
public:
  MirrorObject(int width, int height, int depth, unsigned char strength);
  ~MirrorObject();
  virtual bool update(unsigned interval);
  void setMirrorArea(Vec2i points[4], RoomObject* room);
  void setWallMirror(Vec2i offset, bool positionDependent);
  virtual void realize();
protected:
  void renderCharacter(CharacterObject* chr);
  Vec2i mPoints[4];
  CGE::VertexBuffer* mPolygon;
  RoomObject* mRoom;
  Vec2i mMirrorOffset;
  bool mPositionDependent;
  unsigned char mOpacity;
  int mMirrorCenter;
  bool mIsWallMirror;
};

}

#endif
