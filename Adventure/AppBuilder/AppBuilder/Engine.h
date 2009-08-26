#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <list>
#include <wx/stopwatch.h>
#include "AdvDoc.h"
#include "EngineObjects.h"
#include "Script.h"
#include "Animator.h"

class Engine{
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc) {mData = doc;}
  void render();
  void initGame();
  void exitGame();
  void insertToBlit(BlitObject* obj);
  void setCursorPos(Vec2i pos);
  void leftClick(Vec2i pos);
  bool loadRoom(std::string name);
  bool setFocus(std::string charname);
  std::string resolveFilename(ResourceID id, std::string resource);
  int getWalkGridSize() {return mWalkGridSize;}
protected:
  Engine();
  static Engine* mInstance;
  AdvDocument* mData;
  //rendering
  std::list<BlitObject*> mBlitQueue;
  short mVerts[8];
  //data
  std::vector<RoomObject*> mRooms;
  Object2D* mCursor;
  CharacterObject* mFocussedChar;
  //grid variables
  int mWalkGridSize;
  Vec2i mWalkFields;
  //helper classes
  PcdkScript* mInterpreter;
  Animator* mAnimator;
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
};

#endif
