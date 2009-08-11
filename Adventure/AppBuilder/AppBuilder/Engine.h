#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <list>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#include "AdvDoc.h"

struct BlitObject{
  Vec2i pos;
  Vec2i size;
  int depth;
  GLuint tex;
};

typedef std::vector<BlitObject*> BlitGroup;

typedef std::vector<BlitGroup> Animation;

typedef std::vector<Animation> BlitObjectState;

struct Object2D{
  int state;
  BlitObjectState blits;
};

class Engine{
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc) {mData = doc;}
  void render();
  void initGame();
  void setCursorPos(Vec2i pos);
  bool loadRoom(std::string name);
protected:
  Engine();
  static Engine* mInstance;
  GLuint genTexture(std::string name, Vec2i& size);
  void blit(int x, int y, int width, int height, GLuint tex);
  void insertToBlit(BlitObject* obj);
  void insertToBlit(BlitGroup& obj);
  //void removeToBlit(BlitObject* obj);
  void resortBlit(BlitObject* obj);
  AdvDocument* mData;
  std::list<BlitObject*> mBlitQueue;
  short mVerts[8];
  Object2D mCursor;
  Vec2i mCursorPos;
};

#endif
