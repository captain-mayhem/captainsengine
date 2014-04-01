#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <list>
#include <image/image.h>
#include "AdvDoc.h"
#include "EngineObjects.h"
#include "Script.h"
#include "Animator.h"
#include "Font.h"
#include "Loader.h"

namespace adv{

#ifndef WIN32
int _stricmp(const char* str1, const char* str2);
#endif

class SaveStateProvider;
class SoundPlayer;
class ParticleEngine;
class PostProcessor;
class ItemObject;
class GuiRoom;

typedef void (*exit_callback)();
typedef void (*set_mouse_callback)(int x, int y);

class Engine{
  friend class SaveStateProvider;
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc);
  void render(unsigned time);
  void initGame(exit_callback exit_cb, set_mouse_callback set_mouse_cb);
  void exitGame();
  GLuint genTexture(const CGE::Image* image, Vec2i& size, Vec2f& scale, const CGE::Image* alphaimage=NULL);
  void beginRendering();
  void endRendering(bool inverse=false);
  void insertToBlit(BaseBlitObject* obj);
  void setCursorPos(Vec2i pos);
  Vec2i getCursorPos();
  void leftClick(const Vec2i& pos);
  void leftRelease(const Vec2i& pos);
  void rightClick(const Vec2i& pos);
  void doubleClick(const Vec2i& pos);
  void mouseWheel(int delta);
  bool loadMainRoom(const std::string& name, ExecutionContext* loadreason, ScreenChange change) {return loadRoom(name, false, loadreason, change, 0);}
  bool loadSubRoom(const std::string& name, ExecutionContext* loadreason, int fading) {return loadRoom(name, true, loadreason, SC_DIRECT, fading);}
  void unloadRoom(RoomObject* room, bool mainroom, bool immedately, ExecutionContext* reason);
  bool setFocus(std::string charname, ExecutionContext* reason);
  void setFocus(CharacterObject* character) {mFocussedChar = character;}
  CGE::Image* getImage(const std::string& name);
  float getWalkGridSize(bool doublewalkmap) {return doublewalkmap ? mWalkGridSize/2.0f : (float)mWalkGridSize;}
  Object2D* getObjectAt(const Vec2i& pos);
  Object2D* getObject(const std::string& name, bool searchInventoryFirst);
  CharacterObject* getCharacter(const std::string& name);
  CharacterObject* extractCharacter(const std::string& name);
  RoomObject* getRoom(const std::string& name);
  PcdkScript* getInterpreter() {return mInterpreter;}
  FontRenderer* getFontRenderer() {return mFonts;}
  void setObjectString(const std::string info) {mObjectInfo = info;}
  void setObjectTooltipString(const std::string& info) {mObjectTooltipInfo = info;}
  void walkTo(CharacterObject* chr, const Vec2i& pos, LookDir dir, float speedFactor);
  Animator* getAnimator() {return mAnimator;}
  SaveStateProvider* getSaver() {return mSaver;}
  Vec2i getResolution() {return mData->getProjectSettings()->resolution;}
  ItemObject* createItem(const std::string& name, int count);
  void setUseObject(const std::string& object, const std::string& objectInfo);
  std::string getUseObjectName() {return mUseObjectName;}
  void setGiveObject(const std::string& object, const std::string& objectInfo);
  std::string getGiveObjectName() {return mGiveObjectName;}
  ExecutionContext* loadScript(Script::Type type, const std::string& name);
  void addUIElement(Object2D* elem, int offset);
  ProjectSettings* getSettings() {return mData->getProjectSettings();}
  void setCommand(const std::string& command, bool deleteLinks);
  std::string getActiveCommand();
  EngineEvent getActiveCommandAsEvent();
  void clearGui();
  RoomObject* getContainingRoom(Object2D* object);
  CharacterObject* loadCharacter(const std::string& instanceName, const std::string& className, ExecutionContext* loadreason);
  void keyPress(int key);
  void keyRelease(int key);
  void keyAscii(char chr);
  int unloadRooms();
  std::string getCharacterClass(const std::string instanceName);
  void showTaskbar(bool show) {mShowTaskbar = show;}
  static unsigned roundToPowerOf2(unsigned x);
  void setScreenChange(ScreenChange sc) {mScreenChange = sc;}
  ScreenChange getScreenChange() {return mScreenChange;}
  void setTextEnabled(bool enable) {mTextEnabled = enable;}
  bool isTextEnabled() {return mTextEnabled;}
  void setFontID(int id) {mFontID = id;}
  int getFontID() {return mFontID;}
  bool isKeyDown(int key);
  bool isKeyPressed(int key);
  void quit();
  void reset();
  void renderUnloadingRoom();
  ParticleEngine* getParticleEngine() {return mParticleEngine;}
  void remove(Object2D* obj);
  void enterText(const String& variable, int maxcharacters, ExecutionContext* suspensionReason);
  int getMouseWheelDelta() {return mWheelCount;}
  void setMouseWheelDelta(int delta) {mWheelCount = delta;}
  void restoreRenderDefaults();
  void forceNotToRenderUnloadingRoom() {mForceNotToRenderUnloadingRoom = true;}
  AdvDocument* getData() {return mData;}
  PostProcessor* getPostProcessor() {return mPostProc;}
  RoomObject* getMainRoom();
  void removeScript(ExecutionContext* ctx);
  void showMouse(bool doIt) {mMouseShown = doIt;}
  void enableMouse(bool doIt) {mMouseEnabled = doIt;}
  void setTimeFactor(float factor, bool isFaded) {mTimeFactor = factor; mTimeFactorFaded = isFaded;}
  float getTimeFactor(bool* isFaded=NULL) {if (isFaded != NULL) *isFaded = mTimeFactorFaded; return mTimeFactor;}
  void triggerScreenchange(ExecutionContext* loadreason, ScreenChange change, bool unload);
  void enableMenu(bool doIt) {mMenuEnabled = doIt;}
  CursorObject* getCursor() {return mCursor;}
  void resetCursor(bool resetInstMouse, bool resetDragging);
  std::string getActionText() {return mActionText;}
  void enableFXShapes(bool doIt) {mFxShapesEnabled = doIt;}
  bool areFXShapesEnabled() {return mFxShapesEnabled;}
  void setScrollSpeed(float speed, bool followChar)  {mScrollSpeed = speed; mCamFollowChar = followChar;}
  float getScrollSpeed() {return mScrollSpeed;}
  bool isSubRoomLoaded() {return mSubRoomLoaded;}
  void insertRoom(RoomObject* room, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading);
  void insertCharacter(CharacterObject* obj, std::string room, Vec2i pos, LookDir dir);
  void changeFocus(std::string charname, ExecutionContext* reason);
  ResLoader* getResLoader() {return &mLoader;}
  void enableTextScene(bool doit);
  void setBlockingSpeaker(CharacterObject* chr);
  CharacterObject* getBlockingSpeaker() {return mBlockingSpeaker;}
  void setMousePosition(int x, int y);
  void disposeCharacter(CharacterObject* character);
  CharacterObject* getCachedCharacter(const String& name) {std::map<String,CharacterObject*>::iterator iter = mCharCache.find(name.toLower()); if (iter == mCharCache.end()) return NULL; return iter->second;}
protected:
  Engine();
  static Engine* mInstance;
  AdvDocument* mData;
  //rendering
  std::vector<std::list<BaseBlitObject*> > mBlitQueues;
  short mVerts[8];
  bool mInitialized;
  Vec2i mScrollOffset;
  //data
  std::list<RoomObject*> mRooms;
  std::list<RoomObject*> mRoomsToUnload;
  RoomObject* mUnloadedRoom;
  CursorObject* mCursor;
  CharacterObject* mFocussedChar;
  std::string mLastFocussedChar;
  bool mMainRoomLoaded;
  bool mSubRoomLoaded;
  GuiRoom* mUI;
  RoomObject* mTaskbar;
  bool mShowTaskbar;
  ScreenChange mScreenChange;
  int mFontID;
  bool mTextEnabled;
  //bool mCharOutOfFocus;
  //grid variables
  int mWalkGridSize;
  Vec2i mWalkFields;
  //helper classes
  PcdkScript* mInterpreter;
  Animator* mAnimator;
  FontRenderer* mFonts;
  SaveStateProvider* mSaver;
  ParticleEngine* mParticleEngine;
  //a*-search
  struct AStarData{
    Vec2i pos;
    float g_score;
    float h_score;
    float f_score;
    Vec2i parent;
    //for closedset - checking position equality
    bool operator<(const AStarData& asd) const{
      return pos < asd.pos;
    }
    //for openset - checking f_score
    bool operator>(const AStarData& asd) const{
      return f_score > asd.f_score;
    }
  };
  struct LoadRoom{
    RoomObject* room;
    ExecutionContext* reason;
    ScreenChange screenchange;
    std::string focusChar;
  };
  bool aStarSearch(CharacterObject* chr, const Vec2i& from, const Vec2i& to, std::list<Vec2i>& path);
  float distance(const Vec2i& x, const Vec2i& y);
  std::list<Vec2i> reconstruct_path(AStarData node, const std::set<AStarData>& data);
  bool loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading);
  void handleDragging(const std::string& object);
  //engine - script communication
  std::string mObjectInfo;
  std::string mObjectTooltipInfo;
  unsigned mActiveCommand;
  unsigned mPrevActiveCommand;
  std::string mUseObjectName;
  std::string mGiveObjectName;
  std::string mLinkObjectInfo;
  Object2D* mCurrentObject;
  Object2D* mClickedObject;
  ExecutionContext* mMainScript;
  bool mKeysDown[256];
  bool mKeysPressed[256];
  int mWheelCount;
  bool mExitRequested;
  bool mResetRequested;
  exit_callback mExitCall;
  set_mouse_callback mSetMouseCall;
  bool mMenuShown;
  ExecutionContext* mSuspender;
  String mTextEnter;
  unsigned mNumCharactersEnter;
  bool mBlinkCursorVisible;
  unsigned mBlinkTimeAccu;
  bool mForceNotToRenderUnloadingRoom;
  LoadRoom mPendingLoadRoom;
  RenderableBlitObject* mRenderedMain;
  PostProcessor* mPostProc;
  bool mMouseShown;
  bool mMouseEnabled;
  float mTimeFactor;
  bool mTimeFactorFaded;
  bool mMenuEnabled;
  bool mFxShapesEnabled;
  Object2D* mDraggingObject;
  std::string mActionText;
  float mScrollSpeed;
  bool mCamFollowChar;
  ResLoader mLoader;
  CharacterObject* mBlockingSpeaker;
  int mBlockingSpeakerCount;
  std::map<String, CharacterObject*> mCharCache;
  std::list<CharacterObject*> mCharsToUnload;
};

}

#endif
