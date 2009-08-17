#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <list>
#include "AdvDoc.h"
#include "EngineObjects.h"

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
  bool loadRoom(std::string name);
  std::string resolveFilename(ResourceID id, std::string resource);
protected:
  Engine();
  static Engine* mInstance;
  AdvDocument* mData;
  std::list<BlitObject*> mBlitQueue;
  short mVerts[8];
  std::vector<Object2D*> mObjects;
  Object2D* mCursor;
  int mWalkGridSize;
  Vec2i mWalkFields;
};

#endif
