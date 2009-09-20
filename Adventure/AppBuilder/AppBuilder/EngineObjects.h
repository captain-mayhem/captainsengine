#ifndef ENGINE_OBJECTS_H
#define ENGINE_OBJECTS_H

#ifdef WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include "AdvDoc.h"
#include "Script.h"

class Animation;

class AnimationEndHandler{
public:
  virtual void animationEnded(Animation* anim)=0;
};

struct Color{
  Color(){
    r = 255; g=255; b=255; a=255;
  }
  Color(unsigned packedcolor){
    r = (packedcolor >> 16);
    g = (packedcolor >> 8) & 0xFF;
    b = packedcolor & 0xFF;
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
  bool exists() {return mBlits.size() > 0;}
  void registerAnimationEndHandler(AnimationEndHandler* handler) {mHandler = handler;}
protected:
  std::vector<BlitGroup*> mBlits;
  unsigned mInterval;
  unsigned mTimeAccu;
  unsigned mCurrFrame;
  AnimationEndHandler* mHandler;
};

class Object2D{
public:
  enum Type{
    OBJECT,
    ROOM,
    CHARACTER,
    CURSOR,
  };
  Object2D(int state, const Vec2i& pos, const Vec2i& size, const std::string& name);
  virtual ~Object2D();
  virtual void render();
  virtual Type getType() {return OBJECT;}
  virtual void setPosition(const Vec2i& pos) {mPos = pos;}
  virtual Vec2i getPosition() {return mPos;}
  virtual void animationBegin(const Vec2i& next) {}
  virtual void animationWaypoint(const Vec2i& prev, const Vec2i& next) {}
  virtual void animationEnd(const Vec2i& prev);
  void addAnimation(Animation* anim) {mAnimations.push_back(anim);}
  Animation* getAnimation();
  bool isHit(const Vec2i& point);
  void setScript(ExecutionContext* script) {mScript = script;}
  ExecutionContext* getScript() {return mScript;}
  void setSuspensionScript(ExecutionContext* script);
  int getState() {return mState;}
  void setState(int state) {mState = state;}
  const std::string& getName() {return mName;}
  virtual Vec2i getSize() {return mSize;}
  virtual void setScrollOffset(const Vec2i& offset) {mScrollOffset = offset;}
  Vec2i getScrollOffset() {return mScrollOffset;}
  virtual void save();
  int getDepth();
protected:
  int mState;
  Vec2i mPos;
  Vec2i mSize;
  Vec2i mScrollOffset;
  std::vector<Animation*> mAnimations;
  ExecutionContext* mScript;
  ExecutionContext* mSuspensionScript;
  std::string mName;
};

class CursorObject : public Object2D{
public:
  CursorObject(const Vec2i& pos);
  ~CursorObject();
  void addAnimation(Animation* anim, int command);
  int getNextCommand();
protected:
  std::vector<int> mCommands;
};

class CharacterObject;

class RoomObject : public Object2D{
public:
  RoomObject(const Vec2i& size, const std::string& name);
  ~RoomObject();
  void setBackground(std::string bg);
  void setWalkmap(const std::vector<std::vector<WMField> >& map){mWalkmap = map;}
  void addObject(Object2D* obj);
  virtual void render();
  virtual Type getType() {return ROOM;}
  virtual void setScrollOffset(const Vec2i& offset);
  CharacterObject* extractCharacter(const std::string& name);
  CharacterObject* findCharacter(const std::string& name);
  bool isWalkable(const Vec2i& pos);
  Object2D* getObjectAt(const Vec2i& pos);
  Object2D* getObject(const std::string& name);
  void update(unsigned interval);
  void setLightingColor(const Color& col) {mLighting->setColor(col);}
  void addWalkmapScript(const Vec2i& wmpos, ExecutionContext* script) {mWalkmapScripts[wmpos] = script;}
  void walkTo(const Vec2i& pos);
  virtual void save();
protected:
  std::vector<Object2D*> mObjects;
  std::vector<std::vector<WMField> > mWalkmap;
  LightingBlitObject* mLighting;
  std::map<Vec2i,ExecutionContext*> mWalkmapScripts;
};

class CharacterObject : public Object2D, public AnimationEndHandler{
public:
  CharacterObject(int state, Vec2i pos, const std::string& name);
  ~CharacterObject();
  virtual void render();
  virtual Type getType() {return CHARACTER;}
  void addBasepoint(Vec2i p, Vec2i size) {mBasePoints.push_back(p); mSizes.push_back(size);}
  virtual void setPosition(const Vec2i& pos);
  virtual Vec2i getPosition();
  void setDepth(int depth);
  virtual void animationBegin(const Vec2i& next);
  virtual void animationWaypoint(const Vec2i& prev, const Vec2i& next);
  virtual void animationEnd(const Vec2i& prev);
  virtual void animationEnded(Animation* anim);
  void setLookDir(LookDir dir);
  LookDir getLookDir();
  void setEndLookDir(LookDir dir) {mDesiredDir = dir;}
  Vec2i getOverheadPos();
  void setTextColor(const Color& col) {mTextColor = col;}
  const Color& getTextColor() {return mTextColor;}
  void setFontID(unsigned id) {mFontID = id;}
  unsigned getFontID() {return mFontID;}
  void setNextState(int state) {mNextState = state;}
  void activateNextState();
  static int calculateState(int currState, bool shouldWalk, bool shouldTalk);
  bool isWalking();
  bool isTalking();
  virtual Vec2i getSize();
protected:
  std::vector<Vec2i> mBasePoints;
  std::vector<Vec2i> mSizes;
  bool mMirror;
  LookDir mDesiredDir;
  Color mTextColor;
  unsigned mFontID;
  int mNextState;
};

#endif
