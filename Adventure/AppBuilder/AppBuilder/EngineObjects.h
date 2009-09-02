#ifndef ENGINE_OBJECTS_H
#define ENGINE_OBJECTS_H

#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#include "AdvDoc.h"
#include "Script.h"

class BlitObject{
public:
  BlitObject(std::string texture, int depth, Vec2i offset);
  BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset);
  virtual ~BlitObject();
  bool operator<(const BlitObject& obj);
  void render(Vec2i pos, bool mirrorx);
  virtual void blit();
  int getDepth() {return mDepth;}
  void setDepth(int depth) {mDepth = depth;}
protected:
  Vec2i mOffset;
  Vec2i mPos;
  Vec2i mSize;
  Vec2f mScale;
  int mDepth;
  GLuint mTex;
  bool mMirrorX;
  bool mDeleteTex;
};

class BlitGroup{
public:
  BlitGroup(std::vector<std::string> textures, std::vector<Vec2i> offsets, int depth);
  BlitGroup(const std::string& texture, const Vec2i& offset, int depth);
  ~BlitGroup();
  void render(Vec2i pos, bool mirrorx);
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
  void render(Vec2i pos, bool mirrorx);
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
  Object2D(int state, const Vec2i& pos, const Vec2i& size);
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
  bool isHit(const Vec2i& point);
  void setScript(PcdkScript::ExecutionContext* script) {mScript = script;}
  PcdkScript::ExecutionContext* getScript() {return mScript;}
protected:
  int mState;
  Vec2i mPos;
  Vec2i mSize;
  std::vector<Animation*> mAnimations;
  PcdkScript::ExecutionContext* mScript;
};

class CharacterObject;

class RoomObject : public Object2D{
public:
  RoomObject(const Vec2i& size);
  ~RoomObject();
  void setBackground(std::string bg);
  void setWalkmap(const std::vector<std::vector<WMField> >& map){mWalkmap = map;}
  void addObject(Object2D* obj);
  virtual void render();
  virtual Type getType() {return ROOM;}
  CharacterObject* extractCharacter(const std::string& name);
  CharacterObject* findCharacter(const std::string& name);
  bool isWalkable(const Vec2i& pos);
  Object2D* getObjectAt(const Vec2i& pos);
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
  void setLookDir(LookDir dir);
  void setEndLookDir(LookDir dir) {mDesiredDir = dir;}
protected:
  std::string mName;
  std::vector<Vec2i> mBasePoints;
  bool mMirror;
  LookDir mDesiredDir;
};

#endif
