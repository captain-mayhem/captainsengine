#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define KEY_BACKSPACE VK_BACK
#define KEY_UP VK_UP
#define KEY_DOWN VK_DOWN
#define KEY_LEFT VK_LEFT
#define KEY_RIGHT VK_RIGHT
#define KEY_ESCAPE VK_ESCAPE
#define KEY_RETURN VK_RETURN
#define KEY_A 'A'
#define KEY_D 'D'
#define KEY_R 'R'
#define KEY_S 'S'
#define KEY_W 'W'
#endif
#ifdef UNIX
#include <X11/keysym.h>
#define KEY_BACKSPACE XK_BackSpace
#define KEY_UP XK_Up
#define KEY_DOWN XK_Down
#define KEY_LEFT XK_Left
#define KEY_RIGHT XK_Right
#define KEY_ESCAPE XK_Escape
#define KEY_RETURN XK_Return
#define KEY_A XK_a
#define KEY_D XK_d
#define KEY_R XK_r
#define KEY_S XK_s
#define KEY_W XK_w
#endif

#include "../system/engine.h"

namespace Input{

class Keyboard{
public:
  static void init();
  inline static Keyboard* instance() {return key_;}
  inline static void release() {SAFE_DELETE(key_);}
  //! set keydown callback
  inline void setKeyDownCB(void (*proc)(int key)) {keyDownCB_ = proc;}
  //! set keyup callback
  inline void setKeyUpCB(void (*proc)(int key)) {keyUpCB_ = proc;}
  //! set keydown callback
  inline void setAsciiCB(void (*proc)(unsigned char key)) {asciiCB_ = proc;}
  void keyDown(int key);
  void keyUp(int key);
  void ascii(unsigned char key);
protected:
  static Keyboard* key_;
  Keyboard();
  //! keydown callback
  void (*keyDownCB_)(int key);
  //! keyup callback
  void (*keyUpCB_)(int key);
  //! ascii callback
  void (*asciiCB_)(unsigned char key);
};

}

#endif
