#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <list>
#include <wx/stopwatch.h>
#include "AdvDoc.h"
#include "EngineObjects.h"
#include "Script.h"
#include "Animator.h"
#include "Font.h"

class SaveStateProvider;

class Engine{
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc);
  void render();
  void initGame();
  void exitGame();
  GLuint genTexture(const wxImage& image, Vec2i& size, Vec2f& scale, const wxImage* alphaimage=NULL);
  void insertToBlit(BaseBlitObject* obj);
  void setCursorPos(Vec2i pos);
  Vec2i getCursorPos();
  void leftClick(const Vec2i& pos);
  void rightClick(const Vec2i& pos);
  bool loadRoom(std::string name, bool isSubRoom);
  void unloadRoom(RoomObject* room);
  bool setFocus(std::string charname);
  wxImage getImage(const std::string& name);
  int getWalkGridSize() {return mWalkGridSize;}
  Object2D* getObjectAt(const Vec2i& pos);
  Object2D* getObject(const std::string& name, bool searchInventoryFirst);
  CharacterObject* getCharacter(const std::string& name);
  RoomObject* getRoom(const std::string& name);
  PcdkScript* getInterpreter() {return mInterpreter;}
  FontRenderer* getFontRenderer() {return mFonts;}
  void setObjectString(const std::string info) {mObjectInfo = info;}
  void walkTo(CharacterObject* chr, const Vec2i& pos, LookDir dir);
  Animator* getAnimator() {return mAnimator;}
  SaveStateProvider* getSaver() {return mSaver;}
  Vec2i getResolution() {return mData->getProjectSettings()->resolution;}
  Object2D* createItem(const std::string& name);
  void setUseObject(Object2D* object, const std::string& objectInfo);
  Object2D* getUseObject() {return mUseObject;}
  void setGiveObject(Object2D* object, const std::string& objectInfo);
  Object2D* getGiveObject() {return mGiveObject;}
  ExecutionContext* loadScript(Script::Type type, const std::string& name);
protected:
  Engine();
  static Engine* mInstance;
  AdvDocument* mData;
  //rendering
  std::list<BaseBlitObject*> mBlitQueue;
  short mVerts[8];
  bool mInitialized;
  Vec2i mScrollOffset;
  //data
  std::list<RoomObject*> mRooms;
  std::list<RoomObject*> mRoomsToUnload;
  CursorObject* mCursor;
  CharacterObject* mFocussedChar;
  std::string mLastFocussedChar;
  std::string mLastFocussedCharRoom;
  ExecutionContext* mInitScript;
  bool mMainRoomLoaded;
  bool mSubRoomLoaded;
  //grid variables
  int mWalkGridSize;
  Vec2i mWalkFields;
  //helper classes
  PcdkScript* mInterpreter;
  Animator* mAnimator;
  FontRenderer* mFonts;
  SaveStateProvider* mSaver;
  //TODO move out into render window
  wxStopWatch mTimer;
  std::list<unsigned> mTimeIntervals;
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
  unsigned mActiveCommand;
  Object2D* mUseObject;
  Object2D* mGiveObject;
  std::string mLinkObjectInfo;
};

#endif
