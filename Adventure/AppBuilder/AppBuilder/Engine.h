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

#ifndef WIN32
void DebugBreak();
int _stricmp(const char* str1, const char* str2);
#endif

namespace adv{

class SaveStateProvider;
class SoundPlayer;
class ParticleEngine;

typedef void (*exit_callback)();

class Engine{
  friend class SaveStateProvider;
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc);
  void render(unsigned time);
  void initGame(exit_callback exit_cb);
  void exitGame();
  GLuint genTexture(const CGE::Image* image, Vec2i& size, Vec2f& scale, const CGE::Image* alphaimage=NULL);
  void beginRendering();
  void endRendering();
  void insertToBlit(BaseBlitObject* obj);
  void setCursorPos(Vec2i pos);
  Vec2i getCursorPos();
  void leftClick(const Vec2i& pos);
  void leftRelease(const Vec2i& pos);
  void rightClick(const Vec2i& pos);
  void doubleClick(const Vec2i& pos);
  void mouseWheel(int delta);
  bool loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason);
  void unloadRoom(RoomObject* room, bool mainroom);
  bool setFocus(std::string charname, ExecutionContext* reason);
  CGE::Image* getImage(const std::string& name);
  int getWalkGridSize() {return mWalkGridSize;}
  Object2D* getObjectAt(const Vec2i& pos);
  Object2D* getObject(const std::string& name, bool searchInventoryFirst);
  CharacterObject* getCharacter(const std::string& name);
  CharacterObject* extractCharacter(const std::string& name);
  RoomObject* getRoom(const std::string& name);
  PcdkScript* getInterpreter() {return mInterpreter;}
  FontRenderer* getFontRenderer() {return mFonts;}
  void setObjectString(const std::string info) {mObjectInfo = info;}
  void setObjectTooltipString(const std::string& info) {mObjectTooltipInfo = info;}
  void walkTo(CharacterObject* chr, const Vec2i& pos, LookDir dir);
  Animator* getAnimator() {return mAnimator;}
  SaveStateProvider* getSaver() {return mSaver;}
  Vec2i getResolution() {return mData->getProjectSettings()->resolution;}
  Object2D* createItem(const std::string& name);
  void setUseObject(const std::string& object, const std::string& objectInfo);
  std::string getUseObjectName() {return mUseObjectName;}
  void setGiveObject(const std::string& object, const std::string& objectInfo);
  std::string getGiveObjectName() {return mGiveObjectName;}
  ExecutionContext* loadScript(Script::Type type, const std::string& name);
  void addUIElement(Object2D* elem, int offset);
  ProjectSettings* getSettings() {return mData->getProjectSettings();}
  void setCommand(const std::string& command, bool deleteLinks);
  std::string getActiveCommand();
  void clearGui();
  RoomObject* getContainingRoom(Object2D* object);
  CharacterObject* loadCharacter(const std::string& instanceName, const std::string& className, bool loadContainingRoom, ExecutionContext* loadreason);
  void keyPress(int key);
  void keyRelease(int key);
  void keyAscii(char chr);
  void unloadRooms();
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
  void enterText(const std::string& variable, int maxcharacters, ExecutionContext* suspensionReason);
  int getMouseWheelDelta() {return mWheelCount;}
  void setMouseWheelDelta(int delta) {mWheelCount = delta;}
  void restoreRenderDefaults();
  void forceNotToRenderUnloadingRoom() {mForceNotToRenderUnloadingRoom = true;}
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
  std::list<Object2D*> mUI;
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
  bool aStarSearch(const Vec2i& from, const Vec2i& to, std::list<Vec2i>& path);
  float distance(const Vec2i& x, const Vec2i& y);
  std::list<Vec2i> reconstruct_path(AStarData node, const std::set<AStarData>& data);
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
  bool mMenuShown;
  ExecutionContext* mSuspender;
  std::string mTextEnter;
  unsigned mNumCharactersEnter;
  bool mBlinkCursorVisible;
  unsigned mBlinkTimeAccu;
  bool mForceNotToRenderUnloadingRoom;
};

}

#endif
