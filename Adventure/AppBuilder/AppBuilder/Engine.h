#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <list>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#include "AdvDoc.h"

struct Object2D{
  Vec2i pos;
  Vec2i size;
  int depth;
  GLuint tex;
};

class Engine{
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc) {mData = doc;}
  void render();
  bool loadRoom(std::string name);
protected:
  Engine();
  static Engine* mInstance;
  GLuint genTexture(std::string name);
  void blit(int x, int y, int width, int height, GLuint tex);
  void insertToBlit(Object2D* obj);
  void resortBlit(Object2D* obj);
  AdvDocument* mData;
  std::list<Object2D*> mBlitQueue;
  short mVerts[8];
};

#endif
