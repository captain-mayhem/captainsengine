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
  void setDepth(int depth) {mDepth = depth;}
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
  void setDepth(int depth);
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
  void setDepth(int depth);
  void start();
  void update(unsigned interval);
protected:
  std::vector<BlitGroup*> mBlits;
  unsigned mInterval;
  unsigned mTimeAccu;
  unsigned mCurrFrame;
};

class Object2D{
public:
  enum Type{
    OBJECT,
    ROOM,
    CHARACTER,
  };
  Object2D(int state, Vec2i pos);
  virtual ~Object2D();
  virtual void render();
  virtual Type getType() {return OBJECT;}
  virtual void setPosition(const Vec2i& pos) {mPos = pos;}
  virtual Vec2i getPosition() {return mPos;}
  virtual void animationBegin(const Vec2i& next) {}
  virtual void animationWaypoint(const Vec2i& prev, const Vec2i& next) {}
  virtual void animationEnd(const Vec2i& prev) {}
  void addAnimation(Animation* anim) {mAnimations.push_back(anim);}
  Animation* getAnimation();
protected:
  int mState;
  Vec2i mPos;
  std::vector<Animation*> mAnimations;
};

class CharacterObject;

class RoomObject : public Object2D{
public:
  RoomObject();
  ~RoomObject();
  void setBackground(std::string bg);
  void setWalkmap(const std::vector<std::vector<WMField> >& map){mWalkmap = map;}
  void addObject(Object2D* obj);
  virtual void render();
  virtual Type getType() {return ROOM;}
  CharacterObject* extractCharacter(const std::string& name);
  bool isWalkable(const Vec2i& pos);
protected:
  std::vector<Object2D*> mObjects;
  std::vector<std::vector<WMField> > mWalkmap;
};

class CharacterObject : public Object2D{
public:
  CharacterObject(int state, Vec2i pos, const std::string& name);
  ~CharacterObject();
  virtual void render();
  virtual Type getType() {return CHARACTER;}
  const std::string& getName() {return mName;}
  void addBasepoint(Vec2i p) {mBasePoints.push_back(p);}
  virtual void setPosition(const Vec2i& pos);
  virtual Vec2i getPosition();
  void setDepth(int depth);
  virtual void animationBegin(const Vec2i& next);
  virtual void animationWaypoint(const Vec2i& prev, const Vec2i& next);
  virtual void animationEnd(const Vec2i& prev);
  //Vec2i calcPosition(const Vec2i& p);
protected:
  std::string mName;
  std::vector<Vec2i> mBasePoints;
  bool mMirror;
};

#endif
