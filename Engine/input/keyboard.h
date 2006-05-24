#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define KEY_ESCAPE VK_ESCAPE
#endif
#ifdef UNIX
#include <X11/keysym.h>
#define KEY_ESCAPE XK_Escape
#endif

#include "../system/engine.h"

namespace Input{

class Keyboard{
public:
  static void init();
  inline static Keyboard* instance() {return key_;}
  inline static void release() {SAFE_DELETE(key_);}
  void keyDown(int key);
  void keyUp(int key);
protected:
  static Keyboard* key_;
  Keyboard();
};

}

#endif
