#ifndef ENGINE_OBJECTS_H
#define ENGINE_OBJECTS_H

#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#include "AdvDoc.h"

class BlitObject{
public:
  BlitObject(std::string texture, int depth, Vec2i offset);
  ~BlitObject();
  bool operator<(const BlitObject& obj);
  void render(Vec2i pos);
  void blit();
  int getDepth() {return mDepth;}
protected:
  void genTexture(const std::string& name);
  Vec2i mOffset;
  Vec2i mPos;
  Vec2i mSize;
  Vec2f mScale;
  int mDepth;
  GLuint mTex;
};

class BlitGroup{
public:
  BlitGroup(std::vector<std::string> textures, std::vector<Vec2i> offsets, int depth);
  BlitGroup(const std::string& texture, const Vec2i& offset, int depth);
  ~BlitGroup();
  void render(Vec2i pos);
protected:
  std::vector<BlitObject*> mBlits;
};

typedef std::vector<BlitGroup*> Animation;

typedef std::vector<Animation> BlitObjectState;

class Object2D{
public:
  Object2D(int state, Vec2i pos);
  ~Object2D();
  void render();
  void setPosition(const Vec2i& pos) {mPos = pos;}
protected:
  int mState;
  Vec2i mPos;
public:
  BlitObjectState blits;
};

class RoomObject{
public:
  void render();
  std::vector<Object2D> mObjects;
};

#endif
