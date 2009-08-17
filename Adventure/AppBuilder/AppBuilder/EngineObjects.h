#ifndef ENGINE_OBJECTS_H
#define ENGINE_OBJECTS_H

#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#include "AdvDoc.h"

class BlitObject{
public:
  BlitObject(std::string texture, int depth, Vec2i offset);
  ~BlitObject();
  bool operator<(const BlitObject& obj);
  void render(Vec2i pos);
  void blit();
  int getDepth() {return mDepth;}
protected:
  void genTexture(const std::string& name);
  Vec2i mOffset;
  Vec2i mPos;
  Vec2i mSize;
  Vec2f mScale;
  int mDepth;
  GLuint mTex;
};

class BlitGroup{
public:
  BlitGroup(std::vector<std::string> textures, std::vector<Vec2i> offsets, int depth);
  BlitGroup(const std::string& texture, const Vec2i& offset, int depth);
  ~BlitGroup();
  void render(Vec2i pos);
protected:
  std::vector<BlitObject*> mBlits;
};

class Animation{
public:
  Animation(float fps);
  Animation(ExtendedFrames& frames, float fps, int depth);
  Animation(Frames& frames, float fps, Vec2i offset, int depth);
  ~Animation();
  void render(Vec2i pos);
protected:
  std::vector<BlitGroup*> mBlits;
  float mFps;
  int mCurrFrame;
};

class Object2D{
public:
  Object2D(int state, Vec2i pos);
  virtual ~Object2D();
  virtual void render();
  void setPosition(const Vec2i& pos) {mPos = pos;}
  void addAnimation(Animation* anim) {mAnimations.push_back(anim);}
protected:
  int mState;
  Vec2i mPos;
  std::vector<Animation*> mAnimations;
};

class RoomObject : public Object2D{
public:
  RoomObject();
  ~RoomObject();
  void setBackground(std::string bg);
  void addObject(Object2D* obj);
  virtual void render();
protected:
  std::vector<Object2D*> mObjects;
};

#endif
