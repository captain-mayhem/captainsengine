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
  Vec2f scale;
  int depth;
  GLuint tex;
};

typedef std::vector<BlitObject*> BlitGroup;

typedef std::vector<BlitGroup> Animation;

typedef std::vector<Animation> BlitObjectState;

struct Object2D{
  ~Object2D(){
    for (unsigned i = 0; i < blits.size(); ++i){
      for (unsigned j = 0; j < blits[i].size(); ++j){
        for (unsigned k = 0; k < blits[i][j].size(); ++k){
          delete blits[i][j][k];
        }
      }
    }
  }
  int state;
  BlitObjectState blits;
};

class EngRoom{
public:
  EngRoom();
  
  std::vector<Object2D*> mObjects;
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
  void exitGame();
  void setCursorPos(Vec2i pos);
  bool loadRoom(std::string name);
protected:
  Engine();
  static Engine* mInstance;
  GLuint genTexture(std::string name, Vec2i& size, Vec2f& texscale);
  void blit(int x, int y, int width, int height, float xscale, float yscale, GLuint tex);
  void insertToBlit(BlitObject* obj);
  void insertToBlit(BlitGroup& obj);
  //void removeToBlit(BlitObject* obj);
  void resortBlit(BlitObject* obj);
  AdvDocument* mData;
  std::list<BlitObject*> mBlitQueue;
  short mVerts[8];
  std::vector<Object2D*> mObjects;
  Object2D* mCursor;
  Vec2i mCursorPos;
};

#endif
