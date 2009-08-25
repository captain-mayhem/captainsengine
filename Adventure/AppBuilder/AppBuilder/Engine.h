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
protected:
  Engine();
  static Engine* mInstance;
  AdvDocument* mData;
  std::list<BlitObject*> mBlitQueue;
  short mVerts[8];
  std::vector<RoomObject*> mRooms;
  Object2D* mCursor;
  int mWalkGridSize;
  Vec2i mWalkFields;
  CharacterObject* mFocussedChar;
  PcdkScript* mInterpreter;
  Animator* mAnimator;
  wxStopWatch mTimer;
};

#endif
