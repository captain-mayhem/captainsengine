#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>

class AdvDocument;

class Engine{
public:
  ~Engine();
  static void init() {mInstance = new Engine();}
  static void deinit() {delete mInstance;}
  static Engine* instance() {return mInstance;}
  void setData(AdvDocument* doc) {mData = doc;}
  bool loadRoom(std::string name);
protected:
  Engine();
  static Engine* mInstance;
  GLuint genTexture(std::string name);
  AdvDocument* mData;
};

#endif
