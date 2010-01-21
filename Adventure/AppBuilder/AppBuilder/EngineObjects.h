#ifndef ENGINE_OBJECTS_H
#define ENGINE_OBJECTS_H

#include "AdvDoc.h"
#include "Script.h"
#include "BlitObjects.h"

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

class Object2D : public AnimationEndHandler{
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
  virtual bool animationEnded(Animation* anim);
  void addAnimation(Animation* anim) {mAnimations.push_back(anim);}
  Animation* getAnimation();
  bool isHit(const Vec2i& point);
  void setScript(ExecutionContext* script) {mScript = script; if (script) script->setOwner(this);}
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
  void addNextState(int state) {mNextStates.push_back(state);}
  void activateNextState();
protected:
  int mState;
  Vec2i mPos;
  Vec2i mSize;
  Vec2i mScrollOffset;
  std::vector<Animation*> mAnimations;
  ExecutionContext* mScript;
  ExecutionContext* mSuspensionScript;
  std::string mName;
  std::list<int> mNextStates;
};

class ButtonObject : public Object2D, public BaseBlitObject{
public:
  ButtonObject(const Vec2i& pos, const Vec2i& size, const std::string& text, int id);
  ~ButtonObject();
  virtual void render();
  virtual void blit();
  void setColors(const Color& background, const Color& border, const Color& highlight);
protected:
  std::string mText;
  Color mBackgroundColor;
  Color mBorderColor;
  Color mHighlightColor;
};

class CursorObject : public Object2D{
public:
  CursorObject(const Vec2i& pos);
  ~CursorObject();
  void addAnimation(Animation* anim, int command);
  int getNextCommand(bool& leftClickRequired);
  int getCurrentCommand();
  void setCommand(int command);
protected:
  std::vector<int> mCommands;
};

class CharacterObject;

class RoomObject : public Object2D{
public:
  RoomObject(int state, const Vec2i& pos, const Vec2i& size, const std::string& name);
  ~RoomObject();
  void setBackground(std::string bg, int depth);
  void setParallaxBackground(const std::string& bg, int depth);
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
  void addWalkmapScript(const Vec2i& wmpos, ExecutionContext* script) {mWalkmapScripts[wmpos] = script; script->setOwner(this);}
  void walkTo(const Vec2i& pos);
  virtual void save();
  InventoryDisplay* getInventory() {return mInventroy;}
  void setInventory(InventoryDisplay* disp);
  virtual void setPosition(const Vec2i& pos);
  bool containsObject(Object2D* object);
protected:
  std::vector<Object2D*> mObjects;
  std::vector<std::vector<WMField> > mWalkmap;
  LightingBlitObject* mLighting;
  std::map<Vec2i,ExecutionContext*> mWalkmapScripts;
  InventoryDisplay* mInventroy;
  Animation* mParallaxBackground;
};

class CharacterObject : public Object2D{
  friend class SaveStateProvider;
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
  void setLookDir(LookDir dir);
  void setLookDir(const Vec2i& dir);
  LookDir getLookDir();
  void setEndLookDir(LookDir dir) {mDesiredDir = dir;}
  Vec2i getOverheadPos();
  void setTextColor(const Color& col) {mTextColor = col;}
  const Color& getTextColor() {return mTextColor;}
  void setFontID(unsigned id) {mFontID = id;}
  unsigned getFontID() {return mFontID;}
  static int calculateState(int currState, bool shouldWalk, bool shouldTalk);
  bool isWalking();
  bool isTalking();
  virtual Vec2i getSize();
  Inventory* getInventory() {return mInventory;}
  virtual void save();
  bool isMirrored() {return mMirror;}
  void setMirrored(bool mirrored) {mMirror = mirrored;}
  void setRoom(const std::string& room) {mRoom = room;}
  std::string getRoom() {return mRoom;}
protected:
  std::vector<Vec2i> mBasePoints;
  std::vector<Vec2i> mSizes;
  bool mMirror;
  LookDir mDesiredDir;
  Color mTextColor;
  unsigned mFontID;
  Inventory* mInventory;
  std::string mRoom;
};

#endif
