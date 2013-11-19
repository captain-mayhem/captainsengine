#ifndef ENGINE_OBJECTS_H
#define ENGINE_OBJECTS_H

#include "AdvDoc.h"
#include "Script.h"
#include "BlitObjects.h"
#include "Particles.h"

namespace adv{

class BlitGroup{
public:
  BlitGroup(std::vector<std::string> textures, std::vector<Vec2i> offsets, int depth);
  BlitGroup(const std::string& texture, const Vec2i& offset, int depth);
  ~BlitGroup();
  BlitGroup* clone();
  void render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize, const Color& color, float rotation);
  void setDepth(int depth);
  int getDepth() {if (mBlits.size() == 0) return 0; return mBlits[0]->getDepth();}
  void setBlendMode(BlitObject::BlendMode mode);
protected:
  BlitGroup() {}
  std::vector<BlitObject*> mBlits;
};

class Animation{
public:
  Animation(float fps);
  Animation(ExtendedFrames& frames, float fps, int depth);
  Animation(Frames& frames, float fps, Vec2i offset, int depth);
  Animation(SimpleFrames& frames, float fps, Vec2i offset, int depth);
  ~Animation();
  Animation* clone();
  void render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize, const Color& color, float rotation);
  void setDepth(int depth);
  int getDepth() {if (mBlits.empty()) return 0; return mBlits[0]->getDepth();}
  void start();
  void update(unsigned interval);
  bool exists() {return this != NULL && mBlits.size() > 0;}
  void registerAnimationEndHandler(AnimationEndHandler* handler) {mHandler = handler;}
  void setBlendMode(BlitObject::BlendMode mode);
protected:
  void executeScript();
  std::vector<BlitGroup*> mBlits;
  std::vector<ExecutionContext*> mScripts;
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
  virtual Object2D* clone();
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
  Animation* getAnimation(int state);
  virtual bool isHit(const Vec2i& point);
  void setScript(ExecutionContext* script) {mScript = script; if (script) script->setOwner(this);}
  ExecutionContext* getScript() {return mScript;}
  void setSuspensionScript(ExecutionContext* script);
  int getState() {return mState;}
  virtual void setState(int state) {mState = state; mNextStates.clear();}
  unsigned getNumDefinedStates();
  const std::string& getName() {return mName;}
  virtual Vec2i getSize() {return mSize*mScale;}
  virtual void setScrollOffset(const Vec2i& offset) {mScrollOffset = offset;}
  Vec2i getScrollOffset() {return mScrollOffset;}
  virtual void save();
  virtual int getDepth();
  void setDepth(int depth);
  void addNextState(int state) {mNextStates.push_back(state);}
  int removeLastNextState();
  void activateNextState();
  void clearNextStates() {mNextStates.clear();}
  virtual void setLightingColor(const Color& col) {mLightingColor = col;}
  virtual Color getLightingColor() {return mLightingColor;}
  void setScale(float scale) {mScale = scale;}
  float getScale() {return mScale;}
  void setUserScale(float scale) {mUserScale = scale;}
  float getUserScale() {return mUserScale;}
  void setRotation(float angle) {mRotAngle = angle;}
  float getRotation() {return mRotAngle;}
  void setLighten(bool lighten);
  virtual void update(unsigned interval);
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
  Color mLightingColor;
  float mScale;
  float mUserScale;
  float mRotAngle;
  int mDepth;
};

class ButtonObject : public Object2D, public BlitObject{
public:
  ButtonObject(const Vec2i& pos, const Vec2i& size, const std::string& text, int id);
  ~ButtonObject();
  virtual void render();
  virtual void blit();
  void setColors(const Color& background, const Color& border, const Color& highlight, const Color& text);
  void setFont(int id, bool highlightText) {mFont = id; mHighlightText = highlightText; mOldHighlighting = false;}
  virtual void setPosition(const Vec2i& pos) {Object2D::mPos = pos; BaseBlitObject::mPos = pos;}
protected:
  std::string mText;
  Color mBackgroundColor;
  Color mBorderColor;
  Color mHighlightColor;
  Color mTextColor;
  int mFont;
  bool mHighlightText;
  bool mOldHighlighting;
};

class CursorObject : public Object2D{
public:
  CursorObject(const Vec2i& pos);
  ~CursorObject();
  void addAnimation(Animation* anim, int command);
  int getNextCommand(bool& leftClickRequired, const Vec2i& pos);
  int getCurrentCommand();
  void setCommand(int command);
  virtual bool isHit(const Vec2i& point) {return true; /*the cursor always hits itself*/}
protected:
  std::vector<int> mCommands;
};

class CharacterObject;

class RoomObject : public Object2D{
public:
  RoomObject(int state, const Vec2i& pos, const Vec2i& size, const std::string& name, const Vec2i& depthmap, bool doublewalkmap);
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
  bool isScriptRunning();
  Object2D* getObjectAt(const Vec2i& pos);
  Object2D* getObject(const std::string& name);
  virtual void update(unsigned interval);
  virtual void setLightingColor(const Color& col) {mLighting->setColor(col);}
  virtual Color getLightingColor() {return mLighting->getColor();}
  void addWalkmapScript(const Vec2i& wmpos, ExecutionContext* script) {mWalkmapScripts[wmpos] = script; script->setOwner(this);}
  void walkTo(const Vec2i& pos);
  virtual void save();
  InventoryDisplay* getInventory() {return mInventroy;}
  void setInventory(InventoryDisplay* disp);
  virtual void setPosition(const Vec2i& pos);
  bool containsObject(Object2D* object);
  Vec2i getScriptPosition(ExecutionContext* wmscript);
  void finishScripts(bool execute);
  float getDepthScale(const Vec2i& pos);
  static float getDepthScale(const Vec2i& pos, int depthStart, int depthEnd, int zoomfactor);
  void setZoomFactor(int factor) {mDepthMap.setZoomFactor(factor);}
  void setOpacity(unsigned char opacity);
  void addMirror(MirrorObject* mirror) {mMirrors.push_back(mirror);}
  void addBarrier(const Vec2i box[4]) {mBarriers.push_back(box);}
  const std::vector<Object2D*>& getObjects() {return mObjects;}
  bool hitsBarriers(const ParticleEngine::Particle& particle);
  void modifyWalkmap(const Vec2i& pos, bool walkable);
  void setDepth(int depth);
  void setFadeout(int time) {mFadeout = time;}
  int getFadeout() {return mFadeout;}
  int getWalkGridSize();
protected:
  class DepthMap {
  public:
    DepthMap();
    void init(Vec2i depthmap, int walkgridsize);
    void setZoomFactor(int factor);
    int scaleStart;
    int scaleStop;
    float minVal;
  };
  std::vector<Object2D*> mObjects;
  std::vector<std::vector<WMField> > mWalkmap;
  LightingBlitObject* mLighting;
  std::map<Vec2i,ExecutionContext*> mWalkmapScripts;
  InventoryDisplay* mInventroy;
  Animation* mParallaxBackground;
  DepthMap mDepthMap;
  std::vector<MirrorObject*> mMirrors;
  std::vector<ParticleEngine::Barrier> mBarriers;
  std::map<Vec2i,bool> mModifiedWalkmap;
  int mFadeout;
  bool mDoubleWalkmap;
};

class CharacterObject : public Object2D{
  friend class SaveStateProvider;
public:
  CharacterObject(Character* chrclass, int state, bool mirror, Vec2i pos, const std::string& name);
  ~CharacterObject();
  void render(bool mirrorY);
  virtual void render() {render(false);}
  virtual Type getType() {return CHARACTER;}
  void addBasepoint(Vec2i p, Vec2i size) {mBasePoints.push_back(p); mSizes.push_back(size);}
  virtual void setPosition(const Vec2i& pos);
  void setPosition(const Vec2i& pos, bool isSpawnPos);
  virtual Vec2i getPosition();
  void setDepth(int depth);
  virtual void animationBegin(const Vec2i& next);
  virtual void animationWaypoint(const Vec2i& prev, const Vec2i& next);
  virtual void animationEnd(const Vec2i& prev);
  virtual void setState(int state);
  void setLookDir(LookDir dir);
  void setLookDir(const Vec2i& dir);
  LookDir getLookDir();
  void setEndLookDir(LookDir dir) {mDesiredDir = dir;}
  Vec2i getOverheadPos();
  void setTextColor(const Color& col) {mTextColor = col;}
  const Color& getTextColor() {return mTextColor;}
  void setFontID(unsigned id) {mFontID = id;}
  unsigned getFontID() {return mFontID;}
  bool isWalking();
  bool isTalking();
  virtual Vec2i getSize();
  Inventory* getInventory() {return mInventory;}
  void setInventory(Inventory* inventory) {mInventory = inventory;}
  virtual void save();
  virtual bool isHit(const Vec2i& point);
  bool isMirrored() {return mMirror;}
  void setRoom(const std::string& room) {mRoom = room;}
  std::string getRoom() {return mRoom;}
  void setLinkObject(Object2D* link);
  virtual void update(unsigned interval);
  void setNoZooming(bool nozooming, bool force = false);
  void setFrozenScale(float scale) {mFrozenScale = scale;}
  float getScaleFactor();
  virtual int getDepth();
  bool isSpawnPos();
  void setWalkSound(SoundPlayer* plyr);
  void abortClick();
  Character* getClass() {return mClass;}
  void setTalking(bool doit) {mTalking = doit; updateState(false, false);}
  void setWalking(bool doit) {mWalking = doit; updateState(false, true);}
  void updateState(bool mirror, bool force);
  virtual bool animationEnded(Animation* anim);
  int getWalkGridSize();
protected:
  static int calculateState(LookDir dir, bool& mirror);
  static int calculateState(int currState, bool shouldWalk, bool shouldTalk, bool mirror=false);
  std::vector<Vec2i> mBasePoints;
  std::vector<Vec2i> mSizes;
  bool mMirror;
  LookDir mDesiredDir;
  Color mTextColor;
  unsigned mFontID;
  Inventory* mInventory;
  std::string mRoom;
  Object2D* mLinkObject;
  Vec2i mLinkOffset;
  bool mNoZooming;
  float mFrozenScale;
  int mIdleTime;
  int mIdleTimeout;
  Vec2i mSpawnPos;
  SoundPlayer* mWalkSound;
  Character* mClass;
  bool mWalking;
  bool mTalking;
  LookDir mLookDir;
};

class ObjectGroup{
  friend std::ostream& operator<<(std::ostream& strm, const ObjectGroup& group);
  friend std::istream& operator>>(std::istream& strm, ObjectGroup& data);
public:
  ObjectGroup(const std::string& name) {mName = name;}
  ~ObjectGroup() {}
  void add(const std::string& obj) {mObjects.push_back(obj);}
  std::string getName() {return mName;}
  std::vector<std::string> getObjects() {return mObjects;}
protected:
  std::string mName;
  std::vector<std::string> mObjects;
};

}

#endif
